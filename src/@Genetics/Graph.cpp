#include <stdexcept>

#include "Graph.h"
#include "Task.h"
#include "Processor.h"

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

	for (tid_t id = 0; id < task_count; id++) {
		task = tasks[id];
		processor = processors[mapping[id]];
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

	for (pid_t pid = 0; pid < processor_count; pid++) {
		processor = processors[pid];

		Task *ancestor = NULL, *successor;

		for (tid_t id = 0; id < task_count; id++) {
			successor = tasks[schedule[id]];

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
	task_vector_t roots;

	for (tid_t id = 0; id < task_count; id++)
		if (tasks[id]->is_root())
			roots.push_back(tasks[id]);

	return roots;
}

task_vector_t Graph::get_leaves() const
{
	task_vector_t leaves;

	for (tid_t id = 0; id < task_count; id++)
		if (tasks[id]->is_leaf())
			leaves.push_back(tasks[id]);

	return leaves;
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

double Graph::calc_asap_duration() const
{
	double duration = 0;
	Task *task;

	for (tid_t id = 0; id < task_count; id++) {
		task = tasks[id];
		if (task->is_leaf())
			duration = std::max(duration, task->asap + task->duration);
	}

	return duration;
}

void Graph::calc_start() const
{
	for (tid_t id = 0; id < task_count; id++)
		if (tasks[id]->is_root())
			tasks[id]->propagate_start(0);
}

void Graph::calc_asap() const
{
	for (tid_t id = 0; id < task_count; id++)
		if (tasks[id]->is_root())
			tasks[id]->propagate_asap(0);
}

void Graph::calc_alap() const
{
	double duration = calc_asap_duration();

	for (tid_t id = 0; id < task_count; id++)
		if (tasks[id]->is_leaf())
			tasks[id]->propagate_alap(duration);
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

	for (tid_t id = 0; id < task_count; id++) {
		task = new Task(nc[id], ceff[id]);
		tasks.push_back(task);
		graph->add_task(task);
	}

	for (tid_t pid = 0; pid < task_count; pid++)
		for (tid_t cid = 0; cid < task_count; cid++)
			if (link[pid][cid]) graph->add_link(tasks[pid], tasks[cid]);

	Processor *processor;
	size_t processor_count = frequency.size();

	for (pid_t id = 0; id < processor_count; id++) {
		processor = new Processor(frequency[id], voltage[id], ngate[id]);
		graph->add_processor(processor);
	}

	return graph;
}
