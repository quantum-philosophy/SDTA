#include "Processor.h"
#include "Architecture.h"

#include <nr3.h>
#include <eigen_sym.h>

#include "utils.h"
#include "Hotspot.h"
#include "Schedule.h"
#include "Graph.h"
#include "Task.h"

class CondensedEquation
{
	protected:

	const size_t processor_count;
	const size_t node_count;

	const double sampling_interval;
	const double ambient_temperature;

	/* Matrix exponent */
	MatDoub K;

	VecDoub sinvC;
	MatDoub G;

	/* Eigenvector decomposition
	 *
	 * K = U * L * UT
	 */
	VecDoub L;
	MatDoub U;
	MatDoub UT;

	MatDoub P;
	MatDoub Q;
	MatDoub Y;

	MatDoub m_temp;
	VecDoub v_temp;

	public:

	CondensedEquation(size_t _processor_count, size_t _node_count,
		double _sampling_interval, double _ambient_temperature,
		const double **conductivity, const double *inv_capacitance);

	/* NOTE: power should be of size (step_count x processor_count) */
	void solve(const double *power, double *temperature, size_t step_count);
};

class LeakageCondensedEquation: public CondensedEquation
{
	static const double tol = 0.01;
	static const size_t maxit = 10;

	Leakage leakage;

	public:

	LeakageCondensedEquation(const processor_vector_t &processors,
		size_t _node_count, double _sampling_interval, double _ambient_temperature,
		const double **conductivity, const double *inv_capacitance);

	/* NOTE: dynamic_power should be of size (step_count x processor_count) */
	size_t solve(const double *dynamic_power,
		double *temperature, double *total_power, size_t step_count);
};

CondensedEquation::CondensedEquation(
	size_t _processor_count, size_t _node_count,
	double _sampling_interval, double _ambient_temperature,
	const double **conductivity, const double *inv_capacitance) :

	processor_count(_processor_count),
	node_count(_node_count),
	sampling_interval(_sampling_interval),
	ambient_temperature(_ambient_temperature)
{
	size_t i, j;

	K.resize(node_count, node_count);
	sinvC.resize(node_count);

	L.resize(node_count);
	U.resize(node_count, node_count);
	UT.resize(node_count, node_count);

	G.resize(node_count, node_count);

	m_temp.resize(node_count, node_count);
	v_temp.resize(node_count);

	/* We have:
	 * C * dT/dt = A * T + B
	 */
	MatDoub &A = K;

	for (i = 0; i < node_count; i++) {
		for (j = 0; j < node_count; j++)
			A[i][j] = -conductivity[i][j];
		sinvC[i] = sqrt(inv_capacitance[i]);
	}

	MatDoub &D = A;

	/* We want to get rid of everything in front of dX/dt,
	 * but at the same time we want to keep the matrix in front of X
	 * symmetric, so we do the following substitution:
	 * Y = C^(1/2) * T
	 * D = C^(-1/2) * A * C^(-1/2)
	 * E = C^(-1/2) * B
	 *
	 * Eventually, we have:
	 * dY/dt = DY + E
	 */
	multiply_diagonal_matrix_matrix(sinvC, A, m_temp);
	multiply_matrix_diagonal_matrix(m_temp, sinvC, D);

	/* Eigenvalue decomposition:
	 * D = U * L * UT
	 *
	 * Where:
	 * L = diag(l0, ..., l(n-1))
	 */
	Symmeig S(D);

	__MEMCPY(&L[0], &S.d[0], node_count);
	__MEMCPY(U[0], S.z[0], node_count * node_count); /* matrix */

	transpose_matrix(U, UT);

	/* Matrix exponential:
	 * K = exp(D * t) = U * exp(L * t) UT
	 */
	for (i = 0; i < node_count; i++) v_temp[i] = exp(sampling_interval * L[i]);
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix(m_temp, UT, K);

	/* Coefficient matrix G:
	 * G = D^(-1) * (exp(D * t) - I) * C^(-1/2) =
	 * = U * diag((exp(t * l0) - 1) / l0, ...) * UT * C^(-1/2)
	 */
	for (i = 0; i < node_count; i++) v_temp[i] = (v_temp[i] - 1) / L[i];
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix_diagonal_matrix(m_temp, UT, sinvC, G);
}

void CondensedEquation::solve(const double *power, double *temperature,
	size_t step_count)
{
	size_t i, j, k;

	P.resize(step_count, node_count);
	Q.resize(step_count, node_count);
	Y.resize(step_count, node_count);

	/* M = diag(1/(1 - exp(m * l0)), ....) */
	for (i = 0; i < node_count; i++)
		v_temp[i] = 1.0 / (1.0 - exp(sampling_interval * step_count * L[i]));

	/* Q(0) = G * B(0) */
	multiply_matrix_incomplete_vector(G, power, processor_count, Q[0]);
	/* P(0) = Q(0) */
	__MEMCPY(P[0], Q[0], node_count);

	for (i = 1; i < step_count; i++) {
		/* Q(i) = G * B(i) */
		multiply_matrix_incomplete_vector(G, power + i * processor_count,
			processor_count, Q[i]);
		/* P(i) = K * P(i-1) + Q(i) */
		multiply_matrix_vector_plus_vector(K, P[i - 1], Q[i], P[i]);
	}

	/* Y(0) = U * M * UT * P(m-1), for M see above ^ */
	multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
	multiply_matrix_matrix_vector(m_temp, UT, P[step_count - 1], Y[0]);

	/* Y(i+1) = K * Y(i) + Q(i) */
	for (i = 1; i < step_count; i++)
		multiply_matrix_vector_plus_vector(K, Y[i - 1], Q[i - 1], Y[i]);

	/* Return back to T from Y:
	 * T = C^(-1/2) * Y
	 *
	 * And do not forget about the ambient temperature.
	 */
	for (i = 0, k = 0; i < step_count; i++)
		for (j = 0; j < processor_count; j++, k++)
			temperature[k] = Y[i][j] * sinvC[j] + ambient_temperature;
}

LeakageCondensedEquation::LeakageCondensedEquation(
	const processor_vector_t &processors, size_t _node_count,
	double _sampling_interval, double _ambient_temperature,
	const double **conductivity, const double *inv_capacitance) :

	CondensedEquation(processors.size(), _node_count,
		_sampling_interval, _ambient_temperature,
		conductivity, inv_capacitance),
	leakage(processors)
{
}

size_t LeakageCondensedEquation::solve(const double *dynamic_power,
	double *temperature, double *total_power, size_t step_count)
{
	size_t i, j, k, it;
	double tmp, error, max_error;

	P.resize(step_count, node_count);
	Q.resize(step_count, node_count);
	Y.resize(step_count, node_count);

	/* M = diag(1/(1 - exp(m * l0)), ....) */
	for (i = 0; i < node_count; i++)
		v_temp[i] = 1.0 / (1.0 - exp(sampling_interval * step_count * L[i]));

	leakage.inject(step_count, dynamic_power, ambient_temperature, total_power);

	/* We come to the iterative part */
	for (it = 0;;) {
		/* Q(0) = G * B(0) */
		multiply_matrix_incomplete_vector(G, total_power, processor_count, Q[0]);
		/* P(0) = Q(0) */
		__MEMCPY(P[0], Q[0], node_count);

		for (i = 1; i < step_count; i++) {
			/* Q(i) = G * B(i) */
			multiply_matrix_incomplete_vector(G, total_power + i * processor_count,
				processor_count, Q[i]);
			/* P(i) = K * P(i-1) + Q(i) */
			multiply_matrix_vector_plus_vector(K, P[i - 1], Q[i], P[i]);
		}

		/* Y(0) = U * M * UT * P(m-1), for M see above ^ */
		multiply_matrix_diagonal_matrix(U, v_temp, m_temp);
		multiply_matrix_matrix_vector(m_temp, UT, P[step_count - 1], Y[0]);

		/* Y(i+1) = K * Y(i) + Q(i) */
		for (i = 1; i < step_count; i++)
			multiply_matrix_vector_plus_vector(K, Y[i - 1], Q[i - 1], Y[i]);

		/* Return back to T from Y:
		 * T = C^(-1/2) * Y
		 *
		 * And do not forget about the ambient temperature. Also perform
		 * the error control.
		 */
		max_error = 0;
		for (i = 0, k = 0; i < step_count; i++)
			for (j = 0; j < processor_count; j++, k++) {
				tmp = Y[i][j] * sinvC[j] + ambient_temperature;

				error = abs(temperature[k] - tmp);
				if (max_error < error) max_error = error;

				temperature[k] = tmp;
			}

		it++;

		if (max_error < tol || it >= maxit) break;

		leakage.inject(step_count, dynamic_power, temperature, total_power);
	}

	return it;
}

/******************************************************************************/

size_t read_config_line(str_pair *table, size_t max, const std::string &line)
{
	size_t count = 0;

	std::string name, value;
	std::stringstream stream(line);

	while (true) {
		stream >> name;

		if (name.empty() || stream.eof() || stream.bad())
			throw std::runtime_error("The configuration stream is bad.");

		stream >> value;

		if (stream.bad())
			throw std::runtime_error("The configuration stream is bad.");

		strcpy(table[count].name, name.c_str());
		strcpy(table[count].value, value.c_str());

		count++;

		if (stream.eof()) break;
	}

	return count;
}

Hotspot::Hotspot(const std::string &floorplan_filename,
	const std::string &config_filename, const std::string &config_line)
{
	config = default_thermal_config();

	if (!config_filename.empty()) {
		str_pair table[MAX_ENTRIES];
		size_t i = read_str_pairs(table, MAX_ENTRIES,
			const_cast<char *>(config_filename.c_str()));
		thermal_config_add_from_strs(&config, table, i);
	}

	if (!config_line.empty()) {
		str_pair table[MAX_ENTRIES];
		size_t i = read_config_line(table, MAX_ENTRIES, config_line);
		thermal_config_add_from_strs(&config, table, i);
	}

	floorplan = read_flp(const_cast<char *>(floorplan_filename.c_str()), FALSE);

	model = alloc_RC_model(&config, floorplan);

	populate_R_model(model, floorplan);
	populate_C_model(model, floorplan);

	node_count = model->block->n_nodes;
	processor_count = floorplan->n_units;

	sampling_interval = config.sampling_intvl;
	ambient_temperature = config.ambient;
}

Hotspot::~Hotspot()
{
	delete_RC_model(model);
	free_flp(floorplan, FALSE);
}

void Hotspot::get_conductance(matrix_t &conductance) const
{
	conductance.resize(node_count, node_count);

	double **b = model->block->b;

	for (size_t i = 0; i < node_count; i++)
		for (size_t j = 0; j < node_count; j++)
			conductance[i][j] = b[i][j];
}

void Hotspot::get_capacitance(vector_t &capacitance) const
{
	capacitance.resize(node_count);

	double *a = model->block->a;

	for (size_t i = 0; i < node_count; i++)
		capacitance[i] = a[i];
}

void Hotspot::get_inversed_capacitance(vector_t &inversed_capacitance) const
{
	inversed_capacitance.resize(node_count);

	double *inva = model->block->inva;

	for (size_t i = 0; i < node_count; i++)
		inversed_capacitance[i] = inva[i];
}

/******************************************************************************/

BasicCondensedEquationHotspot::BasicCondensedEquationHotspot(
	const std::string &_floorplan, const std::string &config,
	const std::string &config_line) :

	Hotspot(_floorplan, config, config_line)
{
	CondensedEquation *equation = new CondensedEquation(
		processor_count, node_count, sampling_interval, ambient_temperature,
		(const double **)model->block->b, model->block->inva);

	condensed_equation = (void *)equation;
}

BasicCondensedEquationHotspot::~BasicCondensedEquationHotspot()
{
	CondensedEquation *equation =
		(CondensedEquation *)condensed_equation;
	__DELETE(equation);
}

void BasicCondensedEquationHotspot::solve(const matrix_t &power, matrix_t &temperature)
{
	temperature.resize(power);

	CondensedEquation *equation = (CondensedEquation *)condensed_equation;
	equation->solve(power.pointer(), temperature.pointer(), power.rows());
}

void BasicCondensedEquationHotspot::solve(const matrix_t &power,
	matrix_t &temperature, matrix_t &total_power)
{
	temperature.resize(power);
	total_power.resize(power);

	__MEMCPY(total_power.pointer(), power.pointer(), power.rows() * power.cols());

	CondensedEquation *equation = (CondensedEquation *)condensed_equation;
	equation->solve(power.pointer(), temperature.pointer(), power.rows());
}

/******************************************************************************/

CondensedEquationHotspot::CondensedEquationHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line) :

	BasicCondensedEquationHotspot(floorplan, config, config_line),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval)
{
}

void CondensedEquationHotspot::solve(const Schedule &schedule,
	matrix_t &temperature, matrix_t &power)
{
	dynamic_power.compute(schedule, power);
	BasicCondensedEquationHotspot::solve(power, temperature);
}

/******************************************************************************/

BasicCondensedEquationLeakageHotspot::BasicCondensedEquationLeakageHotspot(
	const Architecture &architecture, const std::string &floorplan,
	const std::string &config, const std::string &config_line) :

	Hotspot(floorplan, config, config_line)
{
	LeakageCondensedEquation *equation = new LeakageCondensedEquation(
		architecture.get_processors(), node_count, sampling_interval,
		ambient_temperature, (const double **)model->block->b,
		model->block->inva);

	condensed_equation = (void *)equation;
}

BasicCondensedEquationLeakageHotspot::~BasicCondensedEquationLeakageHotspot()
{
	LeakageCondensedEquation *equation =
		(LeakageCondensedEquation *)condensed_equation;
	__DELETE(equation);
}

void BasicCondensedEquationLeakageHotspot::solve(const matrix_t &dynamic_power,
	matrix_t &temperature, matrix_t &total_power)
{
	temperature.resize(dynamic_power);
	total_power.resize(dynamic_power);

	LeakageCondensedEquation *equation =
		(LeakageCondensedEquation *)condensed_equation;

	equation->solve(dynamic_power.pointer(), temperature.pointer(),
		total_power.pointer(), dynamic_power.rows());
}

/******************************************************************************/

CondensedEquationLeakageHotspot::CondensedEquationLeakageHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line) :

	BasicCondensedEquationLeakageHotspot(architecture, floorplan, config, config_line),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval)
{
}

void CondensedEquationLeakageHotspot::solve(const Schedule &schedule,
	matrix_t &temperature, matrix_t &total_power)
{
	matrix_t power;
	dynamic_power.compute(schedule, power);
	BasicCondensedEquationLeakageHotspot::solve(power, temperature, total_power);
}

/******************************************************************************/

BasicSteadyStateHotspot::BasicSteadyStateHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line) :

	Hotspot(floorplan, config, config_line),
	processors(architecture.get_processors()),
	deadline(graph.get_deadline()),
	step_count(ceil(deadline / sampling_interval)), storage(NULL)
{
#ifndef SHALLOW_CHECK
	if (step_count == 0)
		throw std::runtime_error("The number of steps is zero.");
#endif

	type_count = processors[0]->size();

#ifndef SHALLOW_CHECK
	for (size_t i = 1; i < processor_count; i++)
		if (type_count != processors[i]->size())
			throw std::runtime_error("The number of types differs between processors.");
#endif

	const task_vector_t &tasks = graph.get_tasks();
	size_t task_count = tasks.size();

	types.resize(task_count);
	for (size_t i = 0; i < task_count; i++)
		types[i] = tasks[i]->get_type();

	storage = new Slot(type_count);
}

BasicSteadyStateHotspot::~BasicSteadyStateHotspot()
{
	__DELETE(storage);
}

void BasicSteadyStateHotspot::solve(const Schedule &schedule,
	matrix_t &temperature, matrix_t &power)
{
	temperature.resize(step_count, processor_count);
	power.resize(step_count, processor_count);
	power.nullify();

	Event event;
	EventQueue queue(schedule, deadline);

	size_t i, start = 0, end;

	SlotTrace trace(processor_count, -1);

	while (queue.next(event)) {
		end = ceil(event.time / sampling_interval);

#ifndef SHALLOW_CHECK
		if (end > step_count)
			throw std::runtime_error("The event time is out of range.");
#endif

		if (end != start) {
			const double *slot_temperature = get(trace);

			for (i = start; i < end && i < step_count; i++)
				__MEMCPY(temperature[i], slot_temperature, processor_count);

			start = end;
		}

		if (event.id < 0 || event.pid < 0) continue;

		int last_type = trace[event.pid];
		int new_type = types[event.id];

		if (last_type < 0) trace[event.pid] = new_type;
		else if (last_type == new_type) trace[event.pid] = -1;
		else throw std::runtime_error("The order of events is incorrect.");
	}
}

const double *BasicSteadyStateHotspot::get(const SlotTrace &trace)
{
	Slot *slot = storage->find(trace);

	double *power = slot->get_data();

	if (!power) {
		power = compute(trace);
		slot->store(power);
	}

	return power;
}

double *SteadyStateHotspot::compute(const SlotTrace &trace) const
{
	double *power = __ALLOC(node_count);
	double *temperature = __ALLOC(node_count);

	__NULLIFY(power, node_count);

	for (size_t i = 0; i < processor_count; i++) {
		if (trace[i] < 0) continue;
		power[i] = processors[i]->calc_power((unsigned int)trace[i]);
	}

	steady_state_temp(model, power, temperature);

	__FREE(power);

	return temperature;
}

SteadyStateLeakageHotspot::SteadyStateLeakageHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line) :

	BasicSteadyStateHotspot(architecture, graph, floorplan,
		config, config_line),
	leakage(architecture.get_processors())
{
}

double *SteadyStateLeakageHotspot::compute(const SlotTrace &trace) const
{
	size_t i, it;

	double error, max_error;

	double *dynamic_power = __ALLOC(node_count);
	double *total_power = __ALLOC(node_count);

	double *last_temperature = __ALLOC(node_count);
	double *temperature = __ALLOC(node_count);

	__NULLIFY(dynamic_power, node_count);
	__NULLIFY(last_temperature, node_count);

	for (size_t i = 0; i < processor_count; i++) {
		if (trace[i] < 0) continue;
		dynamic_power[i] = processors[i]->calc_power((unsigned int)trace[i]);
	}

	leakage.inject(1, dynamic_power, ambient_temperature, total_power);

	for (it = 0;;) {
		steady_state_temp(model, total_power, temperature);

		max_error = 0;

		for (i = 0; i < processor_count; i++) {
			error = abs(temperature[i] - last_temperature[i]);
			if (max_error < error) max_error = error;
			last_temperature[i] = temperature[i];
		}

		it++;

		if (max_error < tol || it >= maxit) break;

		leakage.inject(1, dynamic_power, temperature, total_power);
	}

	__FREE(dynamic_power);
	__FREE(total_power);
	__FREE(last_temperature);

	return temperature;
}

/******************************************************************************/

PreciseSteadyStateHotspot::PreciseSteadyStateHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line) :

	Hotspot(floorplan, config, config_line),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval)
{
}

void PreciseSteadyStateHotspot::solve(const Schedule &schedule,
	matrix_t &temperature, matrix_t &power)
{
	dynamic_power.compute(schedule, power);
	solve(power, temperature);
}

void PreciseSteadyStateHotspot::solve(const matrix_t &_power, matrix_t &_temperature)
{
	size_t step_count = _power.rows();
	_temperature.resize(_power);

	double *extended_power = __ALLOC(node_count);
	double *extended_temperature = __ALLOC(node_count);

	const double *power = _power.pointer();
	double *temperature = _temperature.pointer();

	__NULLIFY(extended_power, node_count);

	for (size_t i = 0; i < step_count; i++) {
		__MEMCPY(extended_power, power + i * processor_count, processor_count);
		steady_state_temp(model, extended_power, extended_temperature);
		__MEMCPY(temperature + i * processor_count, extended_temperature, processor_count);
	}

	__FREE(extended_power);
	__FREE(extended_temperature);
}

/******************************************************************************/

IterativeHotspot::IterativeHotspot(const std::string &floorplan,
	const std::string &config, const std::string &config_line,
	size_t _max_iterations, double _tolerance) :

	Hotspot(floorplan, config, config_line),
	max_iterations(_max_iterations), tolerance(_tolerance)
{
}

size_t IterativeHotspot::verify(const matrix_t &power,
	const matrix_t &reference_temperature, matrix_t &temperature)
{
	size_t step_count = power.rows();

	if (power.cols() != reference_temperature.cols() ||
		step_count != reference_temperature.rows())
		throw std::runtime_error("The reference temperature is wrong.");

	temperature.resize(power);

	const double *_power = power.pointer();
	double *_temperature = temperature.pointer();
	const double *_reference_temperature = reference_temperature.pointer();

	/* Since Hotspot works with power for all thermal nodes,
	 * and our power is only for the processors, we need to extend it
	 * with zeros for the rest of the thermal nodes.
	 */
	matrix_t extended_power(step_count, node_count);
	double *_extended_power = extended_power.pointer();

	extended_power.nullify();
	for (size_t i = 0; i < step_count; i++)
		__MEMCPY(_extended_power + i * node_count,
			_power + i * processor_count, processor_count);

	return solve(_extended_power, _reference_temperature, _temperature, step_count);
}

size_t IterativeHotspot::solve(double *extended_power,
	const double *reference_temperature, double *temperature, size_t step_count)
{
	double *extended_temperature = __ALLOC(node_count);

	set_temp(model, extended_temperature, config.init_temp);

	size_t iterations, total;
	double delta, square_sum;
	const double *reference;

	total = processor_count * step_count;

	for (iterations = 0; iterations < max_iterations; iterations++) {
		square_sum = 0;

		for (size_t i = 0; i < step_count; i++) {
			compute_temp(model, extended_power + node_count * i,
				extended_temperature, sampling_interval);

			/* Compare with the reference */
			reference = reference_temperature + i * processor_count;
			for (size_t j = 0; j < processor_count; j++) {
				delta = extended_temperature[j] - reference[j];
				square_sum += delta * delta;
			}

			/* Copy the new values */
			__MEMCPY(temperature + i * processor_count,
				extended_temperature, processor_count);
		}

		if (std::sqrt(square_sum / double(total)) < tolerance) {
			iterations++;
			break;
		}
	}

	__FREE(extended_temperature);

	return iterations;
}

/******************************************************************************/

EventQueue::EventQueue(const Schedule &schedule, double _deadline) :
	deadline(_deadline), position(0), length(0)
{
	size_t processor_count = schedule.processors();

	for (pid_t pid = 0; pid < processor_count; pid++) {
		const LocalSchedule &local_schedule = schedule[pid];
		size_t count = local_schedule.size();
		for (size_t i = 0; i < count; i++) {
			const ScheduleItem &item = local_schedule[i];
			events.push_back(Event(pid, item.id, item.start));
			events.push_back(Event(pid, item.id, item.start + item.duration));
			length += 2;
		}
	}

	std::stable_sort(events.begin(), events.end(), EventQueue::compare_events);

	events.push_back(Event(-1, -1, deadline));
	length++;
}
