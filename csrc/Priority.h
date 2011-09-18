#ifndef __PRIORITY_H__
#define __PRIORITY_H__

class Priority
{
	public:

	static priority_t calculate(const Architecture &archtecture, const Graph &graph)
	{
		size_t task_count = graph->size();

		priority_t priority(task_count);
		vector_t mobility = Mobility::calculate(architecture, graph);

#ifndef SHALLOW_CHECK
		if (mobility.size() != task_count)
			throw std::runtime_error("The mobility vector is invalid.");
#endif

		std::vector<std::pair<double, const Task *> > pairs(task_count);

		for (size_t i = 0; i < task_count; i++) {
			pairs[i].first = mobility[i];
			pairs[i].second = graph.tasks[i];
		}

		std::stable_sort(pairs.begin(), pairs.end(),
			Comparator<const Task *>::pair);

		for (size_t i = 0; i < task_count; i++)
			priority[pairs[i].second->id] = i;

		return priority;
	}
};

#endif
