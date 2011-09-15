#ifndef __ARCHITECTURE_H__
#define __ARCHITECTURE_H__

#include "common.h"

class Architecture
{
	template<class CT, class PT, class ST>
	friend class GenericEvolution;

	friend class DynamicPower;
	friend class Hotspot;
	friend std::ostream &operator<< (std::ostream &, const Architecture *);

	public:

	Architecture() : processor_count(0) {}

	void add_processor(Processor *processor);
	void assign_tasks(task_vector_t &tasks, const mapping_t &mapping) const;
	void order_tasks(task_vector_t &tasks, const schedule_t &schedule) const;

	inline size_t size() const { return processor_count; }

	inline const Processor *operator[] (pid_t id) const { return processors[id]; }

	protected:

	processor_vector_t processors;
	size_t processor_count;
};

class ArchitectureBuilder : public Architecture
{
	public:

	ArchitectureBuilder(const std::vector<double> &frequency,
		const std::vector<double> &voltage,
		const std::vector<unsigned long int> &ngate,
		const std::vector<std::vector<unsigned long int> > &nc,
		const std::vector<std::vector<double> > &ceff);
	~ArchitectureBuilder();
};

#endif
