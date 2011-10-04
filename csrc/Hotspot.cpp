#include "Processor.h"
#include "Architecture.h"

#include <nr3.h>
#include <eigen_sym.h>

#include "utils.h"
#include "Hotspot.h"
#include "Schedule.h"
#include "Graph.h"
#include "Task.h"

#include "Helper.h"

class CondensedEquationWithoutLeakage
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

	CondensedEquationWithoutLeakage(size_t _processor_count, size_t _node_count,
		double _sampling_interval, double _ambient_temperature,
		const double **neg_conductivity, const double *inv_capacitance);

	/* NOTE: power should be of size (step_count x processor_count) */
	void solve(const double *power, double *temperature, size_t step_count);
};

class CondensedEquation: public CondensedEquationWithoutLeakage
{
	static const double A = 1.1432e-12;
	static const double B = 1.0126e-14;
	static const double alpha = 466.4029;
	static const double beta = -1224.74083;
	static const double gamma = 6.28153;
	static const double delta = 6.9094;

	/* How to calculate Is?
	 *
	 * Take a look at (all coefficients above are from this paper):
	 * "Temperature and Supply Voltage Aware Performance and Power
	 * Modeling at Microarchitecture Level" (July 2005)
	 *
	 * T = [ 100, 100, 80, 80, 60, 60 ] + 273.15
	 * V = [ 0.95, 1.05, 0.95, 1.05, 0.95, 1.05 ]
	 * Iavg = [ 23.44, 29.56, 19.44, 25.14, 16.00, 21.33 ] * 1e-6
	 * Is = mean(Iavg(i) / favg(T(i), V(i)))
	 *
	 * Where favg is the scaling factor (see calc_scaling).
	 */
	static const double Is = 995.7996;

	static const double tol = 0.01;
	static const size_t maxit = 10;

	std::vector<double> voltage;
	std::vector<unsigned long int> ngate;

	public:

	CondensedEquation(const processor_vector_t &processors,
		size_t _node_count, double _sampling_interval, double _ambient_temperature,
		const double **neg_conductivity, const double *inv_capacitance);

	/* NOTE: dynamic_power should be of size (step_count x processor_count) */
	size_t solve(const double *dynamic_power,
		double *temperature, double *total_power, size_t step_count);

	private:

	void inject_leakage(size_t step_count, const double *dynamic_power,
		const double *temperature, double *total_power) const;

	void inject_leakage(size_t step_count, const double *dynamic_power,
		double temperature, double *total_power) const;
};

CondensedEquationWithoutLeakage::CondensedEquationWithoutLeakage(
	size_t _processor_count, size_t _node_count,
	double _sampling_interval, double _ambient_temperature,
	const double **neg_conductivity, const double *inv_capacitance) :

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
			A[i][j] = -neg_conductivity[i][j];
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

	copy_vector(&L[0], &S.d[0], node_count);
	copy_vector(U[0], S.z[0], node_count * node_count); /* matrix */

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

void CondensedEquationWithoutLeakage::solve(const double *power, double *temperature,
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
	copy_vector(P[0], Q[0], node_count);

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

CondensedEquation::CondensedEquation(
	const processor_vector_t &processors, size_t _node_count,
	double _sampling_interval, double _ambient_temperature,
	const double **neg_conductivity, const double *inv_capacitance) :

	CondensedEquationWithoutLeakage(processors.size(), _node_count,
		_sampling_interval, _ambient_temperature,
		neg_conductivity, inv_capacitance)
{
	size_t count = processors.size();

	voltage.resize(count);
	ngate.resize(count);

	for (size_t i = 0; i < count; i++) {
		voltage[i] = processors[i]->get_voltage();
		ngate[i] = processors[i]->get_ngate();
	}
}

size_t CondensedEquation::solve(const double *dynamic_power,
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

	inject_leakage(step_count, dynamic_power, ambient_temperature, total_power);

	/* We come to the iterative part */
	for (it = 0;;) {
		/* Q(0) = G * B(0) */
		multiply_matrix_incomplete_vector(G, total_power, processor_count, Q[0]);
		/* P(0) = Q(0) */
		copy_vector(P[0], Q[0], node_count);

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

		inject_leakage(step_count, dynamic_power, temperature, total_power);
	}

	return it;
}

void CondensedEquation::inject_leakage(
	size_t step_count, const double *dynamic_power,
	const double *temperature, double *total_power) const
{
	/* Pleak = Ngate * Iavg * Vdd
	 * Iavg(T, Vdd) = Is(T0, V0) * favg(T, Vdd)
	 *
	 * Where the scaling factor:
	 * f(T, Vdd) = A * T^2 * e^((alpha * Vdd + beta)/T) +
	 *   B * e^(gamma * Vdd + delta)
	 *
	 * From:
	 * "Temperature and Supply Voltage Aware Performance and
	 * Power Modeling at Microarchitecture Level"
	 */

	size_t i, j, k;
	double temp, favg, voltage;
	unsigned long int ngate;

	for (i = 0, k = 0; i < step_count; i++) {
		for (j = 0; j < processor_count; j++, k++) {
			temp = temperature[k];
			voltage = this->voltage[j];
			ngate = this->ngate[j];

			favg = A * temp * temp *
				exp((alpha * voltage + beta) / temp) +
				B * exp(gamma * voltage + delta);

			total_power[k] = dynamic_power[k] + ngate * Is * favg * voltage;
		}
	}
}

void CondensedEquation::inject_leakage(
	size_t step_count, const double *dynamic_power,
	double temperature, double *total_power) const
{
	size_t i, j, k;
	double favg, voltage;
	unsigned long int ngate;

	for (i = 0, k = 0; i < step_count; i++) {
		for (j = 0; j < processor_count; j++, k++) {
			voltage = this->voltage[j];
			ngate = this->ngate[j];

			favg = A * temperature * temperature *
				exp((alpha * voltage + beta) / temperature) +
				B * exp(gamma * voltage + delta);

			total_power[k] = dynamic_power[k] + ngate * Is * favg * voltage;
		}
	}
}

BasicHotspot::BasicHotspot(const std::string &floorplan_filename,
	const std::string &config_filename, str_pair *extra_table, size_t tsize)
{
	config = default_thermal_config();

	if (!config_filename.empty()) {
		str_pair table[MAX_ENTRIES];
		size_t i = read_str_pairs(&table[0], MAX_ENTRIES,
			const_cast<char *>(config_filename.c_str()));
		thermal_config_add_from_strs(&config, table, i);
	}

	if (extra_table && tsize)
		thermal_config_add_from_strs(&config, extra_table, tsize);

	floorplan = read_flp(const_cast<char *>(floorplan_filename.c_str()), FALSE);

	model = alloc_RC_model(&config, floorplan);

	populate_R_model(model, floorplan);
	populate_C_model(model, floorplan);
}

BasicHotspot::~BasicHotspot()
{
	delete_RC_model(model);
	free_flp(floorplan, FALSE);
}

HotspotWithoutLeakage::HotspotWithoutLeakage(const std::string &floorplan_filename,
	const std::string &config_filename) :

	BasicHotspot(floorplan_filename, config_filename)
{
	CondensedEquationWithoutLeakage *equation = new CondensedEquationWithoutLeakage(
		floorplan->n_units, model->block->n_nodes, config.sampling_intvl,
		config.ambient, (const double **)model->block->b, model->block->inva);

	condensed_equation = (void *)equation;
}

HotspotWithoutLeakage::~HotspotWithoutLeakage()
{
	CondensedEquationWithoutLeakage *equation =
		(CondensedEquationWithoutLeakage *)condensed_equation;
	__DELETE(equation);
}

void HotspotWithoutLeakage::solve(const matrix_t &power, matrix_t &temperature) const
{
	if (floorplan->n_units != power.cols())
		throw std::runtime_error("The floorplan does not match the given power.");

	temperature.resize(power);

	CondensedEquationWithoutLeakage *equation =
		(CondensedEquationWithoutLeakage *)condensed_equation;
	equation->solve(power.pointer(), temperature.pointer(), power.rows());
}

Hotspot::Hotspot(const std::string &floorplan_filename,
	const std::string &config_filename, const Architecture &_architecture) :

	BasicHotspot(floorplan_filename, config_filename)
{
	CondensedEquation *equation = new CondensedEquation(
		_architecture.get_processors(), model->block->n_nodes, config.sampling_intvl,
		config.ambient, (const double **)model->block->b, model->block->inva);

	condensed_equation = (void *)equation;
}

Hotspot::~Hotspot()
{
	CondensedEquation *equation =
		(CondensedEquation *)condensed_equation;
	__DELETE(equation);
}

size_t Hotspot::solve(const matrix_t &dynamic_power,
	matrix_t &temperature, matrix_t &total_power) const
{
	if (floorplan->n_units != dynamic_power.cols())
		throw std::runtime_error("The floorplan does not match the given power.");

	temperature.resize(dynamic_power);
	total_power.resize(dynamic_power);

	CondensedEquation *equation =
		(CondensedEquation *)condensed_equation;

	return equation->solve(dynamic_power.pointer(), temperature.pointer(),
		total_power.pointer(), dynamic_power.rows());
}

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

	std::sort(events.begin(), events.end(), EventQueue::compare_events);

	events.push_back(Event(-1, -1, deadline));
	length++;
}

SteadyStateHotspot::SteadyStateHotspot(const std::string &floorplan_filename,
	const std::string &config_filename, const Architecture &_architecture,
	const Graph &_graph) :

	BasicHotspot(floorplan_filename, config_filename),
	processors(_architecture.get_processors()), processor_count(processors.size()),
	tasks(_graph.tasks), task_count(_graph.size()),
	deadline(_graph.get_deadline()), storage(NULL)
{
	sampling_interval = config.sampling_intvl;

	node_count = model->block->n_nodes;

#ifndef SHALLOW_CHECK
	if (!processor_count)
		throw std::runtime_error("There are no processors.");

	if (!task_count)
		throw std::runtime_error("There are no tasks.");
#endif

	step_count = ceil(_graph.get_deadline() / sampling_interval);

	type_count = processors[0]->size();

#ifndef SHALLOW_CHECK
	if (step_count == 0)
		throw std::runtime_error("The number of steps is zero.");

	for (size_t i = 1; i < processor_count; i++)
		if (type_count != processors[i]->size())
			throw std::runtime_error("The number of types differs between processors.");
#endif

	storage = new Slot(type_count);
}

SteadyStateHotspot::~SteadyStateHotspot()
{
	__DELETE(storage);
}

void SteadyStateHotspot::solve(const Schedule &schedule,
	matrix_t &temperature)
{
	temperature.resize(step_count, processor_count);

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
				memcpy(temperature[i], slot_temperature,
					sizeof(double) * processor_count);

			start = end;
		}

		if (event.id < 0 || event.pid < 0) continue;

		const Task *task = tasks[event.id];

		if (trace[event.pid] < 0)
			trace[event.pid] = task->get_type();
		else
			trace[event.pid] = -1;
	}
}

double *SteadyStateHotspot::compute(const SlotTrace &trace) const
{
	double *power = __ALLOC(node_count);
	double *temperature = __ALLOC(node_count);

	memset(power, 0, sizeof(double) * node_count);

	for (size_t i = 0; i < processor_count; i++) {
		if (trace[i] < 0) continue;
		power[i] = processors[i]->calc_power((unsigned int)trace[i]);
	}

	steady_state_temp(model, power, temperature);

	__FREE(power);

	return temperature;
}

const double *SteadyStateHotspot::get(const SlotTrace &trace)
{
	Slot *slot = storage->find(trace);

	double *power = slot->get_data();

	if (!power) {
		power = compute(trace);
		slot->store(power);
	}

	return power;
}
