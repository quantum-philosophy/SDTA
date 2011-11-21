#include <algorithm>

#include "Priority.h"
#include "Architecture.h"
#include "Graph.h"
#include "Task.h"
#include "GraphAnalysis.h"

priority_t Priority::mobile(const Architecture &architecture,
	const Graph &graph, const mapping_t &mapping)
{
	size_t i, task_count = graph.size();

	priority_t priority(task_count);
	vector_t mobility;

	if (mapping.empty())
		mobility = GraphAnalysis::average_mobility(
			architecture.get_processors(), graph.get_tasks());
	else
		mobility = GraphAnalysis::precise_mobility(
			architecture.get_processors(), graph.get_tasks(), mapping);

#ifndef SHALLOW_CHECK
	if (mobility.size() != task_count)
		throw std::runtime_error("The mobility vector is invalid.");
#endif

	std::vector<std::pair<double, tid_t> > pairs(task_count);

	for (i = 0; i < task_count; i++) {
		pairs[i].first = mobility[i];
		pairs[i].second = i;
	}

	std::stable_sort(pairs.begin(), pairs.end(),
		Comparator<tid_t>::pair);

	for (i = 0; i < task_count; i++)
		priority[pairs[i].second] = i;

	return priority;
}
