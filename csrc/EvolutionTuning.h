#ifndef __EVOLUTION_TUNING_H__
#define __EVOLUTION_TUNING_H__

struct SelectionTuning
{
	std::string method;
	size_t tournament_size;

	SelectionTuning() :
		method("tournament"),
		tournament_size(3) {}
};

struct CrossoverTuning
{
	std::string method;
	double min_rate;
	double scale;
	double exponent;
	size_t points;

	CrossoverTuning() :
		method("npoint"),
		min_rate(0.5),
		scale(1),
		exponent(0),
		points(2) {}
};

struct MutationTuning
{
	std::string method;
	double min_rate;
	double scale;
	double exponent;

	MutationTuning() :
		method("uniform"),
		min_rate(0.01),
		scale(1),
		exponent(-0.05) {};
};

struct TrainingTuning
{
	std::string method;
	double min_rate;
	double scale;
	double exponent;
	size_t max_lessons;
	size_t stall_lessons;

	TrainingTuning() :
		method("peer"),
		min_rate(0.01),
		scale(1),
		exponent(-0.05),
		max_lessons(50),
		stall_lessons(10) {}
};

class EvolutionTuning
{
	public:

	/* Prepare */
	int repeat;
	double deadline_ratio;
	bool reorder_tasks;
	bool include_mapping;

	/* Target */
	bool multiobjective;

	/* Randomize */
	int seed;

	/* Create */
	double uniform_ratio;
	size_t population_size;

	/* Continue */
	size_t max_generations;
	size_t stall_generations;

	SelectionTuning selection;
	CrossoverTuning crossover;
	MutationTuning mutation;
	TrainingTuning training;

	/* Evolve */
	double elitism_rate;

	/* Output */
	bool verbose;
	std::string dump_evolution;

	EvolutionTuning(const std::string &filename = std::string()) :
		repeat(-1),
		deadline_ratio(1.1),
		reorder_tasks(false),
		include_mapping(false),
		multiobjective(false),
		seed(-1),
		uniform_ratio(0.5),
		population_size(25),
		max_generations(100),
		stall_generations(20),
		elitism_rate(0.5),
		verbose(false),
		dump_evolution("")
	{
		if (!filename.empty()) update(filename);
	}

	void update(const std::string &filename);
	void update(std::istream &stream);

	void display(std::ostream &o) const;
};

std::ostream &operator<< (std::ostream &o, const EvolutionTuning &tuning);

#endif
