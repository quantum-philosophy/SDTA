#ifndef __TUNING_H__
#define __TUNING_H__

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

struct parameter_t
{
	std::string name;
	std::string value;

	parameter_t() {}
	parameter_t(const std::string &_name, const std::string &_value) :
		name(_name), value(_value) {}

	template<class T>
	T convert() const
	{
		std::stringstream stream(value);

		T _value;
		stream >> _value;

		return _value;
	}

	double to_double() const
	{
		return convert<double>();
	}

	int to_int() const
	{
		return convert<int>();
	}

	bool to_bool() const
	{
		return convert<bool>();
	}
};

struct parameters_t: public std::vector<parameter_t>
{
	parameters_t() {}
	parameters_t(const std::string &filename);

	void update(const std::string &filename);
	void update(std::istream &main_stream);
};

struct Tuning
{
	virtual void setup(const parameters_t &parameters) = 0;
	virtual void display(std::ostream &o) const = 0;
};

struct SystemTuning: public Tuning
{
	double deadline_ratio;
	double power_scale;
	double time_scale;
	bool reorder_tasks;
	bool verbose;

	SystemTuning() :
		deadline_ratio(1.1),
		power_scale(1.0),
		time_scale(1.0),
		reorder_tasks(false),
		verbose(false) {}

	void setup(const parameters_t &params);
	void display(std::ostream &o) const;
};

struct SolutionTuning: public Tuning
{
	std::string method;
	size_t max_iterations;
	double tolerance;
	bool warmup;
	std::string hotspot;
	std::string leakage;
	std::string assessment;

	SolutionTuning() :
		method("condensed_equation"),
		max_iterations(100),
		tolerance(0.1),
		warmup(false) {}

	void setup(const parameters_t &params);
	void display(std::ostream &o) const;

	inline bool leak() const
	{
		return !leakage.empty();
	}

	inline bool assess() const
	{
		return !assessment.empty();
	}
};

struct OptimizationTuning: public Tuning
{
	int seed;
	int repeat;
	bool mapping;
	bool multiobjective;

	std::string cache;
	std::string dump;

	OptimizationTuning() :
		seed(-1),
		repeat(-1),
		mapping(false),
		multiobjective(false) {}

	void setup(const parameters_t &params);
	void display(std::ostream &o) const;
};

struct CreationTuning: public Tuning
{
	double uniform_ratio;
	size_t population_size;

	CreationTuning() :
		uniform_ratio(0.5),
		population_size(25) {}

	void setup(const parameters_t &params);
	void display(std::ostream &o) const;
};

struct ContinuationTuning: public Tuning
{
	size_t min_generations;
	size_t max_generations;
	size_t stall_generations;
	double time_limit;

	ContinuationTuning() :
		min_generations(0),
		max_generations(500),
		stall_generations(100),
		time_limit(0) {}

	void setup(const parameters_t &params);
	void display(std::ostream &o) const;
};

struct SelectionTuning: public Tuning
{
	std::string method;
	double ratio;
	size_t tournament_size;
	double ranking_factor;

	SelectionTuning() :
		method("tournament"),
		ratio(1.0),
		tournament_size(3),
		ranking_factor(-0.5) {}

	void setup(const parameters_t &params);
	void display(std::ostream &o) const;
};

struct CrossoverTuning: public Tuning
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

	void setup(const parameters_t &params);
	void display(std::ostream &o) const;
};

struct MutationTuning: public Tuning
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

	void setup(const parameters_t &params);
	void display(std::ostream &o) const;
};

struct TrainingTuning: public Tuning
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

	void setup(const parameters_t &params);
	void display(std::ostream &o) const;
};

struct ReplacementTuning: public Tuning
{
	std::string method;
	double elitism_rate;

	ReplacementTuning() :
		method("elitism"),
		elitism_rate(1) {}

	void setup(const parameters_t &params);
	void display(std::ostream &o) const;
};

class EvolutionTuning: public Tuning
{
	public:

	SystemTuning system;
	SolutionTuning solution;
	OptimizationTuning optimization;
	CreationTuning creation;
	ContinuationTuning continuation;
	SelectionTuning selection;
	CrossoverTuning crossover;
	MutationTuning mutation;
	TrainingTuning training;
	ReplacementTuning replacement;

	void setup(const parameters_t &params);
	void display(std::ostream &o) const;
};

std::ostream &operator<< (std::ostream &o, const Tuning &tuning);

#endif
