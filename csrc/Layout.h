#ifndef __LAYOUT_H__
#define __LAYOUT_H__

class Layout
{
	public:

	static layout_t calculate(const Architecture &architecture,
		const Graph &graph)
	{
		size_t processor_count = architecture.size();
		size_t task_count = graph.size();

		layout_t layout(task_count);

		for (size_t i = 0; i < task_count; i++)
#ifdef RANDOM_MAPPING
			layout[i] = Random::number(processor_count);
#else
			layout[i] = i % processor_count;
#endif

		return layout;
	}
};

#endif
