#include "Constrain.h"
#include "Graph.h"
#include "Task.h"
#include "Architecture.h"
#include "Processor.h"

constrains_t Constrain::calculate(const Architecture &architecture,
	const Graph &graph)
{
	size_t task_count = graph.size();
	size_t processor_count = architecture.size();

	const Task *task;
	size_t dependent_count, dependency_count;

	constrains_t constrains(2 * task_count);

	for (tid_t id = 0; id < task_count; id++) {
		task = graph[id];

		constrains[id].peers = bit_string_t(2 * task_count, false);
		constrains[task_count + id].peers = bit_string_t(2 * task_count, false);

		/* Scheduling constrains */
#ifndef UNCONSTRAINED
		bit_string_t dependents(task_count, false);
		dependent_count = count_dependents(task, dependents);

		bit_string_t dependencies(task_count, false);
		dependency_count = count_dependencies(task, dependencies);

		for (int i = 0; i < task_count; i++)
			if (i != id && !dependents[i] && !dependencies[i])
				constrains[id].peers[i] = true;

		constrains[id].min = dependency_count;
		constrains[id].max = task_count - dependent_count;
#else
		constrains[id].min = 0;
		constrains[id].max = task_count:
#endif

		/* Mapping constrains */
		constrains[task_count + id].min = 0;
		constrains[task_count + id].max = processor_count;
	}

	return constrains;
}

size_t Constrain::count_dependents(const Task *task, bit_string_t &counted)
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

size_t Constrain::count_dependencies(const Task *task, bit_string_t &counted)
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
