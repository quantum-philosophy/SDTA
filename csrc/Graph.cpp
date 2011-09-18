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

void Graph::set_constrains()
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
		<< "  Deadline: " << graph->deadline << std::endl;

	o	<< "  " << std::setw(4) << "id" << " -> [ children ]" << std::endl;

	for (tid_t id = 0; id < graph->task_count; id++)
		o << "  " << graph->tasks[id];

	o << "  Rank constrains: "
		<< print_t<constrain_t>(graph->get_constrains()) << std::endl;

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

	finalize();
}

GraphBuilder::~GraphBuilder()
{
	size_t task_count = tasks.size();
	for (size_t i = 0; i < task_count; i++)
		delete tasks[i];
}
