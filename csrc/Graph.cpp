#include <stdexcept>
#include <iomanip>

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

std::ostream &operator<< (std::ostream &o, const Graph *graph)
{
	o	<< "Task Graph:" << std::endl
		<< "  Number of tasks: " << graph->task_count << std::endl
		<< std::setprecision(2) << std::setiosflags(std::ios::fixed)
		<< "  Deadline: " << graph->deadline << std::endl;

	o	<< "  " << std::setw(4) << "id" << " -> [ children ]" << std::endl;

	for (tid_t id = 0; id < graph->task_count; id++)
		o << "  " << graph->tasks[id];

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
