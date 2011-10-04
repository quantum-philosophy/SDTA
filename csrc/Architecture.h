#ifndef __ARCHITECTURE_H__
#define __ARCHITECTURE_H__

#include "common.h"

class Architecture
{
	friend class Graph;

	template<class PT>
	friend class ListScheduler;

	template<class CT, class PT, class ST>
	friend class Evolution;

	friend class DynamicPower;

	friend std::ostream &operator<< (std::ostream &, const Architecture *);

	public:

	Architecture() : processor_count(0) {}

	void add_processor(Processor *processor);

	inline size_t size() const
	{
		return processor_count;
	}

	inline const Processor *operator[] (pid_t id) const
	{
		return processors[id];
	}

	inline const processor_vector_t &get_processors() const
	{
		return processors;
	}

	protected:

	processor_vector_t processors;
	size_t processor_count;
};

class ArchitectureBuilder: public Architecture
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
