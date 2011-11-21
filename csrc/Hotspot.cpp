#include <sstream>

#include "Processor.h"
#include "Architecture.h"

#include "Hotspot.h"
#include "Schedule.h"
#include "Graph.h"
#include "Task.h"

#ifdef MEASURE_TIME
#include "Helper.h"
#endif

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

#ifdef MEASURE_TIME
	struct timespec begin, end;
	Time::measure(&begin);
#endif

	model = alloc_RC_model(&config, floorplan);

	populate_R_model(model, floorplan);
	populate_C_model(model, floorplan);

#ifdef MEASURE_TIME
	Time::measure(&end);
	model_time = Time::substract(&end, &begin);
#endif

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

CondensedEquationHotspot::CondensedEquationHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line) :

	Hotspot(floorplan, config, config_line),
	equation(processor_count, node_count, sampling_interval, ambient_temperature,
		(const double **)model->block->b, model->block->a),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval)
{
#ifdef MEASURE_TIME
	decomposition_time = equation.decomposition_time;
#endif
}

void CondensedEquationHotspot::solve(
	const matrix_t &power, matrix_t &temperature)
{
	temperature.resize(power);
	equation.solve(power, temperature, power.rows());
}

void CondensedEquationHotspot::solve(const Schedule &schedule,
	matrix_t &temperature, matrix_t &power)
{
	dynamic_power.compute(schedule, power);
	solve(power, temperature);
}

/******************************************************************************/

LeakageCondensedEquationHotspot::LeakageCondensedEquationHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line, const Leakage &leakage) :

	Hotspot(floorplan, config, config_line),
	equation(processor_count, node_count, sampling_interval,
		ambient_temperature, model->block->b, model->block->a, leakage),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval)
{
#ifdef MEASURE_TIME
	decomposition_time = equation.decomposition_time;
#endif
}

void LeakageCondensedEquationHotspot::solve(const matrix_t &dynamic_power,
	matrix_t &temperature, matrix_t &total_power)
{
	temperature.resize(dynamic_power);
	total_power.resize(dynamic_power);
	equation.solve(dynamic_power, temperature, total_power, dynamic_power.rows());
}

void LeakageCondensedEquationHotspot::solve(const Schedule &schedule,
	matrix_t &temperature, matrix_t &total_power)
{
	matrix_t power;
	dynamic_power.compute(schedule, power);
	solve(power, temperature, total_power);
}

/******************************************************************************/

FixedCondensedEquationHotspot::FixedCondensedEquationHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line) :

	Hotspot(floorplan, config, config_line),
	equation(processor_count, node_count,
		NUMBER_OF_STEPS(graph.get_deadline(), sampling_interval),
		sampling_interval, ambient_temperature,
		(const double **)model->block->b, model->block->a),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval)
{
#ifdef MEASURE_TIME
	decomposition_time = equation.decomposition_time;
#endif
}

void FixedCondensedEquationHotspot::solve(
	const matrix_t &power, matrix_t &temperature)
{
	temperature.resize(power);
	equation.solve(power, temperature, power.rows());
}

void FixedCondensedEquationHotspot::solve(const Schedule &schedule,
	matrix_t &temperature, matrix_t &power)
{
	dynamic_power.compute(schedule, power);
	solve(power, temperature);
}

/******************************************************************************/

LeakageFixedCondensedEquationHotspot::LeakageFixedCondensedEquationHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line, const Leakage &leakage) :

	Hotspot(floorplan, config, config_line),
	equation(processor_count, node_count,
		NUMBER_OF_STEPS(graph.get_deadline(), sampling_interval),
		sampling_interval, ambient_temperature,
		model->block->b, model->block->a, leakage),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval)
{
#ifdef MEASURE_TIME
	decomposition_time = equation.decomposition_time;
#endif
}

void LeakageFixedCondensedEquationHotspot::solve(const matrix_t &dynamic_power,
	matrix_t &temperature, matrix_t &total_power)
{
	temperature.resize(dynamic_power);
	total_power.resize(dynamic_power);
	equation.solve(dynamic_power, temperature, total_power, dynamic_power.rows());
}

void LeakageFixedCondensedEquationHotspot::solve(const Schedule &schedule,
	matrix_t &temperature, matrix_t &total_power)
{
	matrix_t power;
	dynamic_power.compute(schedule, power);
	solve(power, temperature, total_power);
}

/******************************************************************************/

CoarseCondensedEquationHotspot::CoarseCondensedEquationHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line) :

	Hotspot(floorplan, config, config_line),
	deadline(graph.get_deadline()),
	equation(processor_count, node_count, (const double **)model->block->b,
		model->block->a, ambient_temperature),
	dynamic_power(architecture.get_processors(), graph.get_tasks(), deadline)
{
#ifdef MEASURE_TIME
	decomposition_time = equation.decomposition_time;
#endif
}

void CoarseCondensedEquationHotspot::solve(const Schedule &schedule,
	matrix_t &temperature, matrix_t &power)
{
	vector_t intervals;
	solve(schedule, intervals, temperature, power);
}

void CoarseCondensedEquationHotspot::solve(const Schedule &schedule,
	vector_t &intervals, matrix_t &temperature, matrix_t &power)
{
	dynamic_power.compute(schedule, intervals, power);
	equation.solve(deadline, intervals, power, temperature);
}

/******************************************************************************/

TransientAnalyticalHotspot::TransientAnalyticalHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line, size_t max_iterations,
	double tolerance, bool warmup) :

	Hotspot(floorplan, config, config_line),
	equation(processor_count, node_count, sampling_interval, ambient_temperature,
		(const double **)model->block->b, model->block->a,
		max_iterations, tolerance, warmup),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval)
{
#ifdef MEASURE_TIME
	decomposition_time = equation.decomposition_time;
#endif
}

/******************************************************************************/

BasicSteadyStateHotspot::BasicSteadyStateHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line) :

	Hotspot(floorplan, config, config_line),
	processors(architecture.get_processors()),
	deadline(graph.get_deadline()),
	step_count(NUMBER_OF_STEPS(deadline, sampling_interval)), storage(NULL)
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
		end = STEP_NUMBER(event.time, sampling_interval);

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

SteadyStateHotspot::SteadyStateHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line) :

	BasicSteadyStateHotspot(architecture, graph, floorplan, config, config_line),
	equation(processor_count, node_count, sampling_interval,
		ambient_temperature, (const double **)model->block->b, model->block->a)
{
#ifdef MEASURE_TIME
	decomposition_time = equation.decomposition_time;
#endif
}

double *SteadyStateHotspot::compute(const SlotTrace &trace)
{
	double *power = __ALLOC(node_count);
	double *temperature = __ALLOC(node_count);

	__NULLIFY(power, node_count);

	for (size_t i = 0; i < processor_count; i++) {
		if (trace[i] < 0) continue;
		power[i] = processors[i]->calc_power((unsigned int)trace[i]);
	}

	equation.solve(power, temperature);

	__FREE(power);

	return temperature;
}

LeakageSteadyStateHotspot::LeakageSteadyStateHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line, const Leakage &_leakage) :

	BasicSteadyStateHotspot(architecture, graph, floorplan, config, config_line),
	equation(processor_count, node_count, sampling_interval, ambient_temperature,
		model->block->b, model->block->a, _leakage)
{
	dynamic_power.resize(node_count);
	total_power.resize(node_count);
#ifdef MEASURE_TIME
	decomposition_time = equation.decomposition_time;
#endif
}

double *LeakageSteadyStateHotspot::compute(const SlotTrace &trace)
{
	double *temperature = __ALLOC(node_count);

	for (size_t i = 0; i < processor_count; i++) {
		if (trace[i] < 0) dynamic_power[i] = 0;
		else dynamic_power[i] = processors[i]->calc_power((unsigned int)trace[i]);
	}

	(void)equation.solve(dynamic_power, temperature, total_power);

	return temperature;
}

/******************************************************************************/

PreciseSteadyStateHotspot::PreciseSteadyStateHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line, bool one_step) :

	Hotspot(floorplan, config, config_line),
	equation(processor_count, node_count, sampling_interval,
		ambient_temperature, (const double **)model->block->b, model->block->a),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		one_step ? sampling_interval : graph.get_deadline(), sampling_interval)
{
#ifdef MEASURE_TIME
	decomposition_time = equation.decomposition_time;
#endif
}

/******************************************************************************/

LeakagePreciseSteadyStateHotspot::LeakagePreciseSteadyStateHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line, const Leakage &_leakage,
	bool one_step) :

	Hotspot(floorplan, config, config_line),
	equation(processor_count, node_count, sampling_interval, ambient_temperature,
		model->block->b, model->block->a, _leakage),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		one_step ? sampling_interval : graph.get_deadline(), sampling_interval)
{
#ifdef MEASURE_TIME
	decomposition_time = equation.decomposition_time;
#endif
}

/******************************************************************************/

IterativeHotspot::IterativeHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line, size_t _max_iterations,
	double _tolerance, bool _warmup) :

	Hotspot(floorplan, config, config_line),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval),
	max_iterations(_max_iterations), tolerance(_tolerance), warmup(_warmup)
{
}

void IterativeHotspot::solve(const matrix_t &power, matrix_t &temperature)
{
	size_t step_count = power.rows();

	temperature.resize(step_count, processor_count);

	const double *_power = power;
	double *_temperature = temperature;

	if (power.cols() == node_count) {
		(void)solve(const_cast<double *>(_power), _temperature, step_count);
	}
	else {
		/* Since Hotspot works with power for all thermal nodes,
		 * and our power is only for the processors, we need to extend it
		 * with zeros for the rest of the thermal nodes.
		 */
		matrix_t extended_power(step_count, node_count);
		double *_extended_power = extended_power;

		for (size_t i = 0; i < step_count; i++)
			__MEMCPY(_extended_power + i * node_count,
				_power + i * processor_count, processor_count);

		(void)solve(_extended_power, _temperature, step_count);
	}
}

size_t IterativeHotspot::solve_fixed_iterations(
	double *extended_power, double *temperature, size_t step_count)
{
	double *extended_temperature = __ALLOC(node_count);

	int p;
	size_t i, iterations;

	initialize(extended_power, extended_temperature, step_count);

	for (iterations = 0, p = -1; iterations < max_iterations; iterations++) {
		for (i = 0; i < step_count; i++, p = (p + 1) % step_count) {
			if (p >= 0)
				compute_temp(model, extended_power + node_count * p,
					extended_temperature, sampling_interval);

			__MEMCPY(temperature + i * processor_count,
				extended_temperature, processor_count);
		}
	}

	__FREE(extended_temperature);

	return iterations;
}

size_t IterativeHotspot::solve_error_control(
	double *extended_power, double *temperature, size_t step_count)
{
	double *extended_temperature = __ALLOC(node_count);

	int p;
	size_t i, j, k, iterations;
	double error, delta;

	initialize(extended_power, extended_temperature, step_count);

	for (iterations = 0, p = -1; iterations < max_iterations; iterations++) {
		error = 0;

		for (i = 0, k = 0; i < step_count; i++, p = (p + 1) % step_count) {
			if (p >= 0)
				compute_temp(model, extended_power + node_count * p,
					extended_temperature, sampling_interval);

			for (j = 0; j < processor_count; j++, k++) {
				delta = std::abs(temperature[k] - extended_temperature[j]);
				if (error < delta) error = delta;
				temperature[k] = extended_temperature[j];
			}
		}

		if (error < tolerance) {
			iterations++;
			break;
		}
	}

	__FREE(extended_temperature);

	return iterations;
}

void IterativeHotspot::initialize(const double *extended_power,
	double *extended_temperature, size_t step_count)
{
	int i, j;

	if (warmup) {
		double *total_power = __ALLOC(node_count);

		__NULLIFY(total_power, node_count);

		for (i = 0; i < processor_count; i++) {
			for (j = 0; j < step_count; j++)
				total_power[i] += extended_power[j * node_count + i];
			total_power[i] /= double(step_count);
		}

		steady_state_temp(model, total_power, extended_temperature);

		__FREE(total_power);
	}
	else {
		set_temp(model, extended_temperature, config.init_temp);
	}
}

size_t IterativeHotspot::verify(const matrix_t &power, matrix_t &temperature,
	const matrix_t &reference)
{
	size_t step_count = power.rows();

	temperature.resize(step_count, processor_count);

	const double *_power = power;

	if (power.cols() == node_count) {
		return verify(const_cast<double *>(_power), temperature, step_count,
			reference);
	}
	else {
		matrix_t extended_power(step_count, node_count);
		double *_extended_power = extended_power;

		for (size_t i = 0; i < step_count; i++)
			__MEMCPY(_extended_power + i * node_count,
				_power + i * processor_count, processor_count);

		return verify(_extended_power, temperature, step_count,
			reference);
	}
}

size_t IterativeHotspot::verify(double *extended_power,
	double *temperature, size_t step_count, const double *reference)
{
	double *extended_temperature = __ALLOC(node_count);

	int p;
	size_t i, j, k, iterations;
	double min = DBL_MAX, max = -DBL_MAX, error, delta;

	for (i = 0; i < step_count; i++)
		for (j = 0; j < processor_count; j++) {
			min = std::min(reference[i * processor_count + j], min);
			max = std::max(reference[i * processor_count + j], max);
		}

	initialize(extended_power, extended_temperature, step_count);

	for (iterations = 0, p = -1; iterations < max_iterations; iterations++) {
		error = 0;

		for (i = 0, k = 0; i < step_count; i++, p = (p + 1) % step_count) {
			if (p >= 0)
				compute_temp(model, extended_power + node_count * p,
					extended_temperature, sampling_interval);

			for (j = 0; j < processor_count; j++, k++) {
				delta = reference[k] - extended_temperature[j];
				error += delta * delta;
				temperature[k] = extended_temperature[j];
			}
		}

		error = std::sqrt(error / double(step_count * processor_count)) / (max - min);

		if (error < tolerance) {
			iterations++;
			break;
		}
	}

	__FREE(extended_temperature);

	return iterations;
}

/******************************************************************************/

LeakageIterativeHotspot::LeakageIterativeHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line, size_t _max_iterations,
	double _tolerance, bool _warmup, const Leakage &_leakage) :

	Hotspot(floorplan, config, config_line),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval),
	max_iterations(_max_iterations), tolerance(_tolerance), warmup(_warmup),
	leakage(_leakage)
{
}

void LeakageIterativeHotspot::solve(const matrix_t &dynamic_power,
	matrix_t &temperature, matrix_t &total_power)
{
	size_t step_count = dynamic_power.rows();

	temperature.resize(dynamic_power);
	total_power.resize(dynamic_power);

	const double *_dynamic_power = dynamic_power;
	double *_temperature = temperature;
	double *_total_power = total_power;

	/* Since Hotspot works with power for all thermal nodes,
	 * we need to pass it an extended version of the total power
	 * and then fit it to the short one.
	 */
	matrix_t extended_total_power(step_count, node_count);
	double *_extended_total_power = extended_total_power;

	(void)solve(_dynamic_power, _temperature, _extended_total_power, step_count);

	for (size_t i = 0; i < step_count; i++)
		__MEMCPY(_total_power + i * processor_count,
			_extended_total_power + i * node_count, processor_count);
}

size_t LeakageIterativeHotspot::solve_fixed_iterations(const double *dynamic_power,
	double *temperature, double *extended_total_power, size_t step_count)
{
	double *extended_temperature = __ALLOC(node_count);

	set_temp(model, extended_temperature, ambient_temperature);

	size_t iterations;

	for (iterations = 0; iterations < max_iterations; iterations++)
		for (size_t i = 0; i < step_count; i++) {
			leakage.inject(extended_temperature, dynamic_power + processor_count * i,
				extended_total_power + node_count * i);

			compute_temp(model, extended_total_power + node_count * i,
				extended_temperature, sampling_interval);

			/* Copy the new values */
			__MEMCPY(temperature + i * processor_count,
				extended_temperature, processor_count);
		}

	__FREE(extended_temperature);

	return iterations;
}

size_t LeakageIterativeHotspot::solve_error_control(const double *dynamic_power,
	double *temperature, double *extended_total_power, size_t step_count)
{
	double *extended_temperature = __ALLOC(node_count);

	set_temp(model, extended_temperature, ambient_temperature);

	size_t i, j, k, iterations;
	double error, max_error;

	for (iterations = 0; iterations < max_iterations; iterations++) {
		max_error = 0;
		for (i = 0, k = 0; i < step_count; i++) {
			leakage.inject(extended_temperature, dynamic_power + processor_count * i,
				extended_total_power + node_count * i);

			compute_temp(model, extended_total_power + node_count * i,
				extended_temperature, sampling_interval);

			for (j = 0; j < processor_count; j++, k++) {
				error = std::abs(temperature[k] - extended_temperature[j]);
				if (max_error < error) max_error = error;
				temperature[k] = extended_temperature[j];
			}
		}

		if (max_error < tolerance) break;
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
