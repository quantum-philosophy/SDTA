#include <stdexcept>
#include "Graph.h"

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

void Graph::add_processor(Processor *processor)
{
	processors.push_back(processor);
	processor_count = processors.size();
	processor->id = processor_count - 1;
}

void Graph::assign_mapping(const mapping_t &mapping)
{
	this->mapping = mapping;
	mapped = true;

	Processor *processor;
	Task *task;

	for (size_t i = 0; i < task_count; i++) {
		task = tasks[i];
		processor = processors[mapping[i]];
		task->map(processor);
	}

	calc_asap();
	calc_alap();
}

void Graph::assign_schedule(const schedule_t &schedule)
{
	if (!mapped) std::runtime_error("The graph should be mapped.");

	this->schedule = schedule;
	scheduled = true;

	Processor *processor;

	for (size_t i = 0; i < processor_count; i++) {
		processor = processors[i];

		Task *ancestor = NULL, *successor;

		for (size_t j = 0; j < task_count; j++) {
			successor = tasks[schedule[j]];

			if (successor->processor != processor) continue;

			successor->map(processor);

			if (ancestor) {
				ancestor->set_successor(successor);
				successor->set_ancestor(ancestor);
			}

			ancestor = successor;
		}
	}
}

task_vector_t Graph::get_roots() const
{
	std::vector<Task *> roots;

	for (size_t i = 0; i < task_count; i++)
		if (tasks[i]->is_root()) roots.push_back(tasks[i]);

	return roots;
}

task_vector_t Graph::get_leaves() const
{
	std::vector<Task *> leaves;

	for (size_t i = 0; i < task_count; i++)
		if (tasks[i]->is_leaf()) leaves.push_back(tasks[i]);

	return leaves;
}

double Graph::calc_duration() const
{
	double duration = 0;
	Task *task;

	for (size_t i = 0; i < task_count; i++) {
		task = tasks[i];
		if (task->is_leaf())
			duration = std::max(duration, task->start + task->duration);
	}

	return duration;
}

double Graph::calc_asap_duration() const
{
	double duration = 0;
	Task *task;

	for (size_t i = 0; i < task_count; i++) {
		task = tasks[i];
		if (task->is_leaf())
			duration = std::max(duration, task->asap + task->duration);
	}

	return duration;
}

void Graph::calc_start() const
{
	for (size_t i = 0; i < task_count; i++)
		if (tasks[i]->is_root()) tasks[i]->propagate_start(0);
}

void Graph::calc_asap() const
{
	for (size_t i = 0; i < task_count; i++)
		if (tasks[i]->is_root()) tasks[i]->propagate_asap(0);
}

void Graph::calc_alap() const
{
	double duration = calc_asap_duration();

	for (size_t i = 0; i < task_count; i++)
		if (tasks[i]->is_leaf()) tasks[i]->propagate_alap(duration);
}

Graph *Graph::build(std::vector<unsigned long int> &nc,
	std::vector<double> &ceff, std::vector<std::vector<bool> > &link,
	std::vector<double> &frequency, std::vector<double> &voltage,
	std::vector<unsigned long int> &ngate)
{
	Graph *graph = new Graph;

	Task *task;
	size_t task_count = nc.size();
	task_vector_t tasks;

	for (size_t i = 0; i < task_count; i++) {
		task = new Task(nc[i], ceff[i]);
		tasks.push_back(task);
		graph->add_task(task);
	}

	for (size_t i = 0; i < task_count; i++)
		for (size_t j = 0; j < task_count; j++)
			if (link[i][j]) graph->add_link(tasks[i], tasks[j]);

	Processor *processor;
	size_t processor_count = frequency.size();

	for (size_t i = 0; i < processor_count; i++) {
		processor = new Processor(frequency[i], voltage[i], ngate[i]);
		graph->add_processor(processor);
	}

	return graph;
}
