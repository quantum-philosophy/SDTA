#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <limits>

#include "Task.h"
#include "Graph.h"
#include "Processor.h"
#include "Architecture.h"
#include "Hotspot.h"
#include "Lifetime.h"
#include "DynamicPower.h"

void Graph::add_task(Task *task)
{
	tasks.push_back(task);
	task_count = tasks.size();
	task->id = task_count - 1;
}

void Graph::add_link(Task *parent, Task *child)
{
	parent->add_child(child);
	child->add_parent(parent);
}

void Graph::assign_mapping(const Architecture *architecture,
	const mapping_t &mapping)
{
	this->architecture = architecture;
	assign_mapping(mapping);
}

void Graph::assign_mapping(const mapping_t &mapping)
{
	if (!architecture)
		throw std::runtime_error("The architecture is unknown.");

	if (mapping.size() != task_count)
		throw std::runtime_error("The given mapping is not sufficient.");

	this->mapping = mapping;

	architecture->assign_tasks(tasks, mapping);

	calc_constrains();
}

void Graph::assign_schedule(const schedule_t &schedule)
{
#ifndef SHALLOW_CHECK
	if (!architecture) std::runtime_error("The graph should be mapped.");

	if (schedule.size() != task_count)
		throw std::runtime_error("The given schedule is not sufficient.");
#endif

	this->schedule = schedule;

	/* Should reset the start times of the tasks */
	architecture->order_tasks(tasks, schedule);

	/* ... so that we can calculate new start times */
	calc_start();

	/* ... and the duration of the whole graph */
	duration = calc_duration();
}

double Graph::calc_duration() const
{
	double duration = 0;
	Task *task;

	for (tid_t id = 0; id < task_count; id++) {
		task = tasks[id];
		if (task->is_leaf())
			duration = std::max(duration, task->start + task->duration);
	}

	return duration;
}

void Graph::calc_start() const
{
	for (tid_t id = 0; id < task_count; id++)
		if (tasks[id]->is_root())
			tasks[id]->propagate_start(0);
}

price_t Graph::evaluate(Hotspot *hotspot) const
{
	double sampling_interval = hotspot->sampling_interval();

	matrix_t dynamic_power, temperature, total_power;

	DynamicPower::compute(this, sampling_interval, dynamic_power);

	(void)hotspot->solve(architecture,
		dynamic_power, temperature, total_power);

	double lifetime = Lifetime::predict(temperature, sampling_interval);

	size_t total_count = total_power.cols() * total_power.rows();
	const double *ptr = total_power.pointer();

	double energy = 0;
	for (int i = 0; i < total_count; i++, ptr++) energy += *ptr;
	energy *= sampling_interval;

	return price_t(lifetime, energy);
}

void Graph::reorder_tasks(const schedule_t &order)
{
	size_t i;

	/* First of all, reorder the tasks themselves */
	task_vector_t new_tasks(task_count);

	for (i = 0; i < task_count; i++) {
		new_tasks[i] = tasks[order[i]];
		new_tasks[i]->id = i;
	}

	tasks = new_tasks;

	/* Fix the mapping */
	if (!mapping.empty()) {
		mapping_t new_mapping(task_count);

		for (i = 0; i < task_count; i++)
			new_mapping[i] = mapping[order[i]];

		mapping = new_mapping;
	}

	/* Fix the schedule */
	if (!schedule.empty()) {
		schedule_t new_schedule(task_count);

		for (i = 0; i < task_count; i++)
			new_schedule[i] = schedule[order[i]];

		schedule = new_schedule;
	}

	/* Fix the constrains */
	if (!constrains.empty()) {
		constrains_t new_constrains(task_count);

		/* TODO: reorder the peers as well */
		for (i = 0; i < task_count; i++)
			new_constrains[i] = constrains[order[i]];

		constrains = new_constrains;
	}
}

layout_t Graph::calc_layout(const Architecture *architecture) const
{
	if (!architecture) architecture = this->architecture;

#ifndef SHALLOW_CHECK
	if (!architecture)
		throw std::runtime_error("The graph should be mapped.");
#endif

	size_t processor_count = architecture->size();

	layout_t layout(task_count);

	for (size_t i = 0; i < task_count; i++)
#ifdef RANDOM_MAPPING
		layout[i] = Random::number(processor_count);
#else
		layout[i] = i % processor_count;
#endif

	return layout;
}

layout_t Graph::calc_layout(const schedule_t &schedule,
	const Architecture *architecture) const
{
	if (!architecture) architecture = this->architecture;

#ifndef SHALLOW_CHECK
	if (!architecture)
		throw std::runtime_error("The graph should be mapped.");

	if (schedule.size() != task_count)
		throw std::runtime_error("The schedule size is invalid.");
#endif

	pid_t pid;
	size_t i, j, processor_count = architecture->size();

	layout_t layout(task_count);

	vector_t processor_time(processor_count, 0);
	const processor_vector_t &processors = architecture->processors;

	for (i = 0; i < task_count; i++) {
		const Task *task = tasks[schedule[i]];

		for (pid = 0, j = 1; j < processor_count; j++)
			if (processor_time[j] < processor_time[pid]) pid = j;

		layout[schedule[i]] = pid;
		processor_time[pid] += processors[pid]->calc_duration(task->type);
	}

	return layout;
}

vector_t Graph::calc_mobility() const
{
	tid_t id;
	const Task *task;
	vector_t asap(task_count, -1);
	vector_t alap(task_count, std::numeric_limits<double>::max());
	vector_t mobility(task_count);

	/* Calculate ASAP */
	for (id = 0; id < task_count; id++) {
		task = tasks[id];
		if (task->is_root())
			task->collect_asap(asap, 0);
	}

	/* Calculate the overall duration according to ASAP */
	double duration = 0;
	for (id = 0; id < task_count; id++) {
		task = tasks[id];
		if (task->is_leaf())
			duration = std::max(duration, asap[id] + task->duration);
	}

	/* Calculate ALAP */
	for (id = 0; id < task_count; id++) {
		task = tasks[id];
		if (task->is_leaf())
			task->collect_alap(alap, duration);
	}

	double epsilon = std::numeric_limits<double>::epsilon();
	for (size_t i = 0; i < task_count; i++) {
		mobility[i] = alap[i] - asap[i];
		if (mobility[i] < epsilon) mobility[i] = 0;
	}

	return mobility;
}

priority_t Graph::calc_priority() const
{
	priority_t priority(task_count);
	vector_t mobility = calc_mobility();

#ifndef SHALLOW_CHECK
	if (mobility.size() != task_count)
		throw std::runtime_error("The mobility vector is invalid.");
#endif

	std::vector<std::pair<double, const Task *> > pairs(task_count);

	for (size_t i = 0; i < task_count; i++) {
		pairs[i].first = mobility[i];
		pairs[i].second = tasks[i];
	}

	std::stable_sort(pairs.begin(), pairs.end(),
		Comparator<const Task *>::pair);

	for (size_t i = 0; i < task_count; i++)
		priority[pairs[i].second->id] = i;

	return priority;
}

priority_t Graph::calc_depth_priority() const
{
	tid_t id;
	const Task *task;

	priority_t priority(task_count, -1);

	for (id = 0; id < task_count; id++) {
		task = tasks[id];
		if (task->is_root())
			task->collect_depth(priority, 0);
	}

	return priority;
}

void Graph::calc_constrains()
{
	const Task *task;
	size_t dependent_count, dependency_count, processor_count;

	constrains = constrains_t(2 * task_count);
	processor_count = architecture->size();

	for (tid_t id = 0; id < task_count; id++) {
		task = tasks[id];

#ifndef UNCONSTRAINED
		bit_string_t dependents(task_count, false);
		dependent_count = count_dependents(task, dependents);

		bit_string_t dependencies(task_count, false);
		dependency_count = count_dependencies(task, dependencies);

		/* Scheduling constrains */
		constrains[id].peers.clear();
		constrains[id].min = dependency_count;
		constrains[id].max = task_count - dependent_count - 1;

		for (int i = 0; i < task_count; i++)
			if (i != id && !dependents[i] && !dependencies[i])
				constrains[id].peers.push_back(i);
#else
		constrains[id].peers.clear();
		constrains[id].min = 0;
		constrains[id].max = task_count - 1;
#endif

		/* Mapping constrains */
		constrains[task_count + id].peers.clear();
		constrains[task_count + id].min = 0;
		constrains[task_count + id].max = processor_count - 1;
	}
}

size_t Graph::count_dependents(const Task *task) const
{
	bit_string_t counted(task_count, false);
	return count_dependents(task, counted);
}

size_t Graph::count_dependents(const Task *task, bit_string_t &counted) const
{
	size_t child_count, dependents = 0;

	child_count = task->children.size();

	for (size_t i = 0; i < child_count; i++) {
		if (counted[task->children[i]->id]) continue;
		counted[task->children[i]->id] = true;
		dependents += 1 + count_dependents(task->children[i], counted);
	}

	return dependents;
}

size_t Graph::count_dependencies(const Task *task) const
{
	bit_string_t counted(task_count, false);
	return count_dependencies(task, counted);
}

size_t Graph::count_dependencies(const Task *task, bit_string_t &counted) const
{
	size_t parent_count, dependencies = 0;

	parent_count = task->parents.size();

	for (size_t i = 0; i < parent_count; i++) {
		if (counted[task->parents[i]->id]) continue;
		counted[task->parents[i]->id] = true;
		dependencies += 1 + count_dependencies(task->parents[i], counted);
	}

	return dependencies;
}

std::ostream &operator<< (std::ostream &o, const Graph *graph)
{
	o	<< "Task Graph:" << std::endl
		<< "  Number of tasks: " << graph->task_count << std::endl
		<< std::setprecision(2) << std::setiosflags(std::ios::fixed)
		<< "  Duration: " << graph->duration << std::endl
		<< "  Deadline: " << graph->deadline << std::endl;

	o	<< "  "
		<< std::setw(4) << "id" << " ( "
		<< std::setw(4) << "proc" << " : "
		<< std::setw(8) << "start" << " : "
		<< std::setw(8) << "duration" << " ) -> [ children ]" << std::endl;

	for (tid_t id = 0; id < graph->task_count; id++)
		o << "  " << graph->tasks[id];

	o << "  Rank constrains: "
		<< print_t<constrain_t>(graph->constrains) << std::endl;

	return o;
}

GraphBuilder::GraphBuilder(const std::vector<unsigned int> &type,
	const std::vector<std::vector<bool> > &link) : Graph()
{
	Task *task;
	task_vector_t tasks;

	size_t task_count = type.size();

	if (task_count == 0)
		throw std::runtime_error("There are no tasks specified.");

	if (link.size() != task_count)
		throw std::runtime_error("The link matrix is not consistent.");

	for (size_t i = 0; i < task_count; i++)
		if (link[i].size() != task_count)
			throw std::runtime_error("The link matrix is not consistent.");

	for (tid_t id = 0; id < task_count; id++) {
		task = new Task(type[id]);
		tasks.push_back(task);
		add_task(task);
	}

	for (tid_t pid = 0; pid < task_count; pid++) {
		for (tid_t cid = 0; cid < task_count; cid++)
			if (link[pid][cid])
				add_link(tasks[pid], tasks[cid]);
	}
}

GraphBuilder::~GraphBuilder()
{
	size_t task_count = tasks.size();
	for (size_t i = 0; i < task_count; i++)
		delete tasks[i];
}
