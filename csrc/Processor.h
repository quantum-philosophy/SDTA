#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#include "common.h"

class Processor
{
	friend class Architecture;
	friend class Task;

	friend std::ostream &operator<< (std::ostream &, const Processor *);
	friend std::ostream &operator<< (std::ostream &, const Task *);

	pid_t id;
	double frequency;
	double voltage;
	unsigned long int ngate;

	std::vector<unsigned long int> nc;
	std::vector<double> ceff;
	size_t type_count;

	public:

	Processor(double _frequency, double _voltage, unsigned long int _ngate) :
		id(-1), frequency(_frequency), voltage(_voltage), ngate(_ngate),
		type_count(0) {}

	void add_type(unsigned long int nc, double ceff);

	inline size_t size() const
	{
		return type_count;
	}

	inline double get_voltage() const
	{
		return voltage;
	}

	inline unsigned long int get_ngate() const
	{
		return ngate;
	}

	inline double calc_duration(unsigned int type) const
	{
#ifndef SHALLOW_CHECK
		if (type >= type_count)
			throw std::runtime_error("The processor does not have such type.");
#endif
		/* t = NC / f */
		return nc[type] / frequency;
	}

	inline double calc_power(unsigned int type) const
	{
#ifndef SHALLOW_CHECK
		if (type >= type_count)
			throw std::runtime_error("The processor does not have such type.");
#endif
		/* P = Ceff * f * V^2 */
		return ceff[type] * frequency * voltage * voltage;
	}
};

#endif
