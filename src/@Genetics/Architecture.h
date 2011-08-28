#ifndef __ARCHITECTURE_H__
#define __ARCHITECTURE_H__

#include "Common.h"

class Architecture
{
	friend std::ostream &operator<< (std::ostream &, const Architecture *);

	protected:

	processor_vector_t processors;
	size_t processor_count;

	public:

	Architecture() : processor_count(0) {}

	void add_processor(Processor *processor);
	void map(task_vector_t &tasks, const mapping_t &mapping) const;
	void distribute(task_vector_t &tasks, const schedule_t &schedule) const;

	size_t size() const { return processor_count; }

	const Processor *operator[] (pid_t id) const { return processors[id]; }
};

class ArchitectureBuilder : public Architecture
{
	public:

	ArchitectureBuilder(std::vector<double> &frequency,
		std::vector<double> &voltage, std::vector<unsigned long int> &ngate,
		std::vector<std::vector<unsigned long int> > &nc,
		std::vector<std::vector<double> > &ceff);
	~ArchitectureBuilder();
};

#endif
