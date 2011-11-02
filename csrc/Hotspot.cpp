#include <sstream>

#include "Processor.h"
#include "Architecture.h"

#include "Hotspot.h"
#include "Schedule.h"
#include "Graph.h"
#include "Task.h"

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

	Hotspot(_floorplan, config, config_line),
	equation(processor_count, node_count, sampling_interval, ambient_temperature,
		(const double **)model->block->b, model->block->a)
{
}

void BasicCondensedEquationHotspot::solve(
	const matrix_t &power, matrix_t &temperature)
{
	temperature.resize(power);
	equation.solve(power, temperature, power.rows());
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

BasicLeakageCondensedEquationHotspot::BasicLeakageCondensedEquationHotspot(
	const Architecture &architecture, const std::string &floorplan,
	const std::string &config, const std::string &config_line,
	const Leakage &leakage) :

	Hotspot(floorplan, config, config_line),
	equation(processor_count, node_count, sampling_interval,
		ambient_temperature, model->block->b, model->block->a, leakage)
{
}

void BasicLeakageCondensedEquationHotspot::solve(const matrix_t &dynamic_power,
	matrix_t &temperature, matrix_t &total_power)
{
	temperature.resize(dynamic_power);
	total_power.resize(dynamic_power);
	equation.solve(dynamic_power, temperature, total_power, dynamic_power.rows());
}

/******************************************************************************/

LeakageCondensedEquationHotspot::LeakageCondensedEquationHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line, const Leakage &leakage) :

	BasicLeakageCondensedEquationHotspot(
		architecture, floorplan, config, config_line, leakage),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval)
{
}

void LeakageCondensedEquationHotspot::solve(const Schedule &schedule,
	matrix_t &temperature, matrix_t &total_power)
{
	matrix_t power;
	dynamic_power.compute(schedule, power);
	BasicLeakageCondensedEquationHotspot::solve(power, temperature, total_power);
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
	const std::string &config_line, size_t max_iterations) :

	Hotspot(floorplan, config, config_line),
	equation(processor_count, node_count, sampling_interval, ambient_temperature,
		(const double **)model->block->b, model->block->a, max_iterations),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval)
{
}

void TransientAnalyticalHotspot::solve(
	const matrix_t &power, matrix_t &temperature)
{
	temperature.resize(power);
	equation.solve(power, temperature, power.rows());
}

void TransientAnalyticalHotspot::solve(const Schedule &schedule,
	matrix_t &temperature, matrix_t &power)
{
	dynamic_power.compute(schedule, power);
	solve(power, temperature);
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

LeakageSteadyStateHotspot::LeakageSteadyStateHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line, const Leakage &_leakage) :

	BasicSteadyStateHotspot(architecture, graph, floorplan,
		config, config_line), leakage(_leakage)
{
}

double *LeakageSteadyStateHotspot::compute(const SlotTrace &trace) const
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

	const size_t max_iterations = leakage.get_max_iterations();
	const double tolerance = leakage.get_tolerance();

	leakage.inject(ambient_temperature, dynamic_power, total_power);

	for (it = 1;; it++) {
		steady_state_temp(model, total_power, temperature);

		if (it < max_iterations) {
			/* There is a reason to check the error.
			 */
			max_error = 0;
			for (i = 0; i < processor_count; i++) {
				error = abs(temperature[i] - last_temperature[i]);
				if (max_error < error) max_error = error;
				last_temperature[i] = temperature[i];
			}

			/* Still have some iterations left,
			 * the only question is the error.
			 */
			if (max_error < tolerance) break;
		}
		else {
			/* Limit of iterations is reached,
			 * quite right now.
			 */
			break;
		}

		leakage.inject(temperature, dynamic_power, total_power);
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

	const double *power = _power;
	double *temperature = _temperature;

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

IterativeHotspot::IterativeHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line,  size_t _max_iterations) :

	Hotspot(floorplan, config, config_line),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval),
	max_iterations(_max_iterations)
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

void IterativeHotspot::solve(const Schedule &schedule,
	matrix_t &temperature, matrix_t &power)
{
	dynamic_power.compute(schedule, power);
	solve(power, temperature);
}

size_t IterativeHotspot::solve(double *extended_power,
	double *temperature, size_t step_count)
{
	double *extended_temperature = __ALLOC(node_count);

	set_temp(model, extended_temperature, config.init_temp);

	size_t iterations;

	for (iterations = 0; iterations < max_iterations; iterations++)
		for (size_t i = 0; i < step_count; i++) {
			compute_temp(model, extended_power + node_count * i,
				extended_temperature, sampling_interval);

			/* Copy the new values */
			__MEMCPY(temperature + i * processor_count,
				extended_temperature, processor_count);
		}

	__FREE(extended_temperature);

	return iterations;
}

/******************************************************************************/

LeakageIterativeHotspot::LeakageIterativeHotspot(
	const Architecture &architecture, const Graph &graph,
	const std::string &floorplan, const std::string &config,
	const std::string &config_line, size_t _max_iterations,
	const Leakage &_leakage) :

	Hotspot(floorplan, config, config_line),
	dynamic_power(architecture.get_processors(), graph.get_tasks(),
		graph.get_deadline(), sampling_interval),
	max_iterations(_max_iterations), leakage(_leakage)
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

void LeakageIterativeHotspot::solve(const matrix_t &dynamic_power,
	matrix_t &temperature)
{
	matrix_t total_power;
	solve(dynamic_power, temperature, total_power);
}

void LeakageIterativeHotspot::solve(const Schedule &schedule,
	matrix_t &temperature, matrix_t &total_power)
{
	matrix_t power;
	dynamic_power.compute(schedule, power);
	solve(power, temperature, total_power);
}

size_t LeakageIterativeHotspot::solve(const double *dynamic_power,
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
