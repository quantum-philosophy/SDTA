#ifndef __GA_COMMON_H__
#define __GA_COMMON_H__

typedef double gene_t;
typedef eoReal<gene_t> chromosome_t;

class eoUniformRangeMutation: public eoMonOp<chromosome_t>
{
	gene_t max;
	gene_t min;
	unsigned int points;

	public:

	eoUniformRangeMutation(gene_t _min, gene_t _max, unsigned _points)
		: max(_max), min(_min), points(_points) {}

	eoUniformRangeMutation(gene_t _min, gene_t _max)
		: max(_max), min(_min), points(1) {}

	virtual std::string className() const { return "eoUniformRangeMutation"; }

	bool operator()(chromosome_t& chromosome) {

		unsigned int length = chromosome.size();
		unsigned int point;
		bool hasChanged = false;
		gene_t last;

		for (unsigned int i = 0; i < points; i++) {
			point = rng.random(length);
			last = chromosome[point];
			chromosome[point] = eo::random(min, max);
			if (last != chromosome[point]) hasChanged = true;
		}

		return hasChanged;
	}
};

class eoMatlabMonitor: public eoMonitor
{
	eoPop<chromosome_t> &population;

	public:

	eoMatlabMonitor(eoPop<chromosome_t> &_population) : population(_population) {}

	virtual std::string className() const { return "eoMatlabMonotor"; }

	virtual eoMonitor& operator()(void)
	{
		return *this;
	}
};

#endif
