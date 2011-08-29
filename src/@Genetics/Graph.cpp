#include <stdexcept>
#include <iomanip>

#include "Task.h"
#include "Graph.h"
#include "Processor.h"
#include "Architecture.h"

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
	if (mapping.size() != task_count)
		throw std::runtime_error("The given mapping is not sufficient.");

	this->architecture = architecture;
	this->mapping = mapping;

	architecture->map(tasks, mapping);

	calc_asap();
	calc_alap();
}

void Graph::assign_schedule(const schedule_t &schedule)
{
	if (!architecture) std::runtime_error("The graph should be mapped.");

	if (schedule.size() != task_count)
		throw std::runtime_error("The given schedule is not sufficient.");

	this->schedule = schedule;

	architecture->distribute(tasks, schedule);

	calc_start();
	duration = calc_duration();
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

std::ostream &operator<< (std::ostream &o, const Graph *graph)
{
	o	<< "Task Graph: " << std::endl
		<< "  Number of tasks: " << graph->task_count << std::endl
		<< std::setprecision(2) << std::setiosflags(std::ios::fixed)
		<< "  Duration: " << graph->duration << std::endl
		<< "  Deadline: " << graph->deadline << std::endl;

	o	<< "  "
		<< std::setw(4) << "id" << " ( "
		<< std::setw(4) << "proc" << " : "
		<< std::setw(4) << "type" << " : "
		<< std::setw(8) << "start" << " : "
		<< std::setw(8) << "duration" << " : "
		<< std::setw(8) << "asap" << " : "
		<< std::setw(8) << "mobility" << " : "
		<< std::setw(8) << "alap" << " ) -> [ "
		<< "children" << " ] " << std::endl;

	for (tid_t id = 0; id < graph->task_count; id++)
		o << "  " << graph->tasks[id];

	if (!graph->schedule.empty())
		o << "  Schedule: " << graph->schedule;

	return o;
}

GraphBuilder::GraphBuilder(std::vector<unsigned int> &type,
	std::vector<std::vector<bool> > &link) : Graph()
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
	for (size_t i = 0; i < task_count; i++)
		delete tasks[i];
}
