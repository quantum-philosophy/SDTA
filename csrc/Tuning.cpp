#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "Tuning.h"
#include "Helper.h"

parameters_t::parameters_t(const std::string &filename)
{
	update(filename);
}

void parameters_t::update(const std::string &filename)
{
	std::ifstream file(filename.c_str());

	if (!file.is_open())
		throw std::runtime_error("Cannot open the tuning file.");

	update(file);
}

void parameters_t::update(std::istream &main_stream)
{
	main_stream.exceptions(std::ios::failbit | std::ios::badbit);

	std::string line, name, value;

	while (true) {
		try {
			std::getline(main_stream, line);
		}
		catch (...) {
			break;
		}

		Helper::chomp(line);

		if (line.empty() || line[0] == '#') continue;

		std::stringstream stream(line);
		stream.exceptions(std::ios::failbit | std::ios::badbit);

		stream >> name;

		try {
			std::getline(stream, value);
		}
		catch (...) {
			value = "";
		}

		Helper::chomp(value);

		push_back(parameter_t(name, value));
	}
}

void SystemTuning::setup(const parameters_t &params)
{
	for (parameters_t::const_iterator it = params.begin();
		it != params.end(); it++) {

		if (it->name == "deadline_ratio")
			deadline_ratio = it->to_double();
		if (it->name == "max_temperature")
			max_temperature = it->to_double();
		else if (it->name == "power_scale")
			power_scale = it->to_double();
		else if (it->name == "time_scale")
			time_scale = it->to_double();
		else if (it->name == "homogeneous")
			homogeneous = it->to_bool();
		else if (it->name == "reorder_tasks")
			reorder_tasks = it->to_bool();
		else if (it->name == "verbose")
			verbose = it->to_bool();
	}
}

void SystemTuning::display(std::ostream &o) const
{
	o
		<< "System:" << std::endl
		<< "  Deadline ratio:       " << deadline_ratio << std::endl
		<< "  Maximal temperature:  " << max_temperature << std::endl
		<< "  Task power scale:     " << power_scale << std::endl
		<< "  Task time scale:      " << time_scale << std::endl
		<< "  Homogeneous:          " << homogeneous << std::endl
		<< "  Reorder tasks:        " << reorder_tasks << std::endl
		<< "  Verbose:              " << verbose << std::endl;
}

void SolutionTuning::setup(const parameters_t &params)
{
	for (parameters_t::const_iterator it = params.begin();
		it != params.end(); it++) {

		if (it->name == "solution")
			method = it->value;
		else if (it->name == "max_iterations")
			max_iterations = it->to_int();
		else if (it->name == "tolerance")
			tolerance = it->to_double();
		else if (it->name == "warmup")
			warmup = it->to_bool();
		else if (it->name == "hotspot")
			hotspot = it->value;
		else if (it->name == "leakage")
			leakage = it->value;
		else if (it->name == "assessment")
			assessment = it->value;
	}
}

void SolutionTuning::display(std::ostream &o) const
{
	o
		<< "Solution:               " << method << std::endl
		<< "  Max iterations:       " << max_iterations << std::endl
		<< std::setprecision(4)
		<< "  Tolerance:            " << tolerance << std::endl
		<< "  Warm up:              " << warmup << std::endl
		<< "  Hotspot:              " << hotspot << std::endl
		<< "  Leakage:              " << leakage << std::endl
		<< "  Assessment:           " << assessment << std::endl;
}

void OptimizationTuning::setup(const parameters_t &params)
{
	for (parameters_t::const_iterator it = params.begin();
		it != params.end(); it++) {

		if (it->name == "seed")
			seed = it->to_int();
		else if (it->name == "repeat")
			repeat = it->to_int();
		else if (it->name == "mapping")
			mapping = it->to_bool();
		else if (it->name == "multiobjective")
			multiobjective = it->to_bool();
		else if (it->name == "cache")
			cache = it->value;
		else if (it->name == "dump")
			dump = it->value;
	}
}

void OptimizationTuning::display(std::ostream &o) const
{
	o
		<< "Optimization:" << std::endl
		<< std::setprecision(0)
		<< "  Seed:                 " << seed << std::endl
		<< "  Repeat:               " << repeat << std::endl
		<< "  Consider mapping:     " << mapping << std::endl
		<< "  Multi-objective:      " << multiobjective << std::endl
		<< "  Cache server:         " << cache << std::endl
		<< "  Dump evolution:       " << dump << std::endl;
}

void CreationTuning::setup(const parameters_t &params)
{
	for (parameters_t::const_iterator it = params.begin();
		it != params.end(); it++) {

		if (it->name == "uniform_ratio")
			uniform_ratio = it->to_double();
		else if (it->name == "population_size")
			population_size = it->to_int();
	}
}

void CreationTuning::display(std::ostream &o) const
{
	o
		<< "Creation:" << std::endl
		<< std::setprecision(3)
		<< "  Uniform ratio:        " << uniform_ratio << std::endl
		<< std::setprecision(0)
		<< "  Population size:      " << population_size << std::endl;
}

void ContinuationTuning::setup(const parameters_t &params)
{
	for (parameters_t::const_iterator it = params.begin();
		it != params.end(); it++) {

		if (it->name == "min_generations")
			min_generations = it->to_int();
		else if (it->name == "max_generations")
			max_generations = it->to_int();
		else if (it->name == "stall_generations")
			stall_generations = it->to_int();
		else if (it->name == "time_limit")
			time_limit = it->to_double();
	}
}

void ContinuationTuning::display(std::ostream &o) const
{
	o
		<< "Continuation:" << std::endl
		<< "  Minimal generations:  " << min_generations << std::endl
		<< "  Maximum generations:  " << max_generations << std::endl
		<< "  Stall generations:    " << stall_generations << std::endl
		<< std::setprecision(2)
		<< "  Time limit:           " << time_limit << std::endl;
}

void SelectionTuning::setup(const parameters_t &params)
{
	for (parameters_t::const_iterator it = params.begin();
		it != params.end(); it++) {

		if (it->name == "selection")
			method = it->value;
		else if (it->name == "selection_ratio")
			ratio = it->to_double();
		else if (it->name == "tournament_size")
			tournament_size = it->to_int();
		else if (it->name == "ranking_factor")
			ranking_factor = it->to_double();
	}
}

void SelectionTuning::display(std::ostream &o) const
{
	o
		<< "Selection:              " << method << std::endl
		<< std::setprecision(2)
		<< "  Ratio:                " << ratio << std::endl
		<< std::setprecision(0)
		<< "  Tournament size:      " << tournament_size << std::endl
		<< std::setprecision(2)
		<< "  Ranking factor:       " << ranking_factor << std::endl;
}

void CrossoverTuning::setup(const parameters_t &params)
{
	for (parameters_t::const_iterator it = params.begin();
		it != params.end(); it++) {

		if (it->name == "crossover")
			method = it->value;
		else if (it->name == "crossover_min_rate")
			min_rate = it->to_double();
		else if (it->name == "crossover_scale")
			scale = it->to_double();
		else if (it->name == "crossover_exponent")
			exponent = it->to_double();
		else if (it->name == "crossover_points")
			points = it->to_int();
	}
}

void CrossoverTuning::display(std::ostream &o) const
{
	o
		<< "Crossover:              " << method << std::endl
		<< std::setprecision(3)
		<< "  Minimal rate:         " << min_rate << std::endl
		<< "  Scale:                " << scale << std::endl
		<< "  Exponent:             " << exponent << std::endl
		<< std::setprecision(0)
		<< "  Points:               " << points << std::endl;
}

void MutationTuning::setup(const parameters_t &params)
{
	for (parameters_t::const_iterator it = params.begin();
		it != params.end(); it++) {

		if (it->name == "mutation")
			method = it->value;
		else if (it->name == "mutation_min_rate")
			min_rate = it->to_double();
		else if (it->name == "mutation_scale")
			scale = it->to_double();
		else if (it->name == "mutation_exponent")
			exponent = it->to_double();
	}
}

void MutationTuning::display(std::ostream &o) const
{
	o
		<< "Mutation:               " << method << std::endl
		<< std::setprecision(3)
		<< "  Minimal rate:         " << min_rate << std::endl
		<< "  Scale:                " << scale << std::endl
		<< "  Exponent:             " << exponent << std::endl;
}

void TrainingTuning::setup(const parameters_t &params)
{
	for (parameters_t::const_iterator it = params.begin();
		it != params.end(); it++) {

		if (it->name == "training")
			method = it->value;
		else if (it->name == "training_min_rate")
			min_rate = it->to_double();
		else if (it->name == "training_scale")
			scale = it->to_double();
		else if (it->name == "training_exponent")
			exponent = it->to_double();
		else if (it->name == "max_lessons")
			max_lessons = it->to_int();
		else if (it->name == "stall_lessons")
			stall_lessons = it->to_int();
	}
}

void TrainingTuning::display(std::ostream &o) const
{
	o
		<< "Training:               " << method << std::endl
		<< std::setprecision(3)
		<< "  Minimal rate:         " << min_rate << std::endl
		<< "  Scale:                " << scale << std::endl
		<< "  Exponent:             " << exponent << std::endl
		<< std::setprecision(0)
		<< "  Maximum lessons:      " << max_lessons << std::endl
		<< "  Stall lessons:        " << stall_lessons << std::endl;
}

void ReplacementTuning::setup(const parameters_t &params)
{
	for (parameters_t::const_iterator it = params.begin();
		it != params.end(); it++) {

		if (it->name == "replacement")
			method = it->value;
		else if (it->name == "elitism_rate")
			elitism_rate = it->to_double();
	}
}

void ReplacementTuning::display(std::ostream &o) const
{
	o
		<< "Replacement:            " << method << std::endl
		<< std::setprecision(2)
		<< "  Elitism rate:         " << elitism_rate << std::endl;
}

void EvolutionTuning::setup(const parameters_t &params)
{
	system.setup(params);
	solution.setup(params);
	optimization.setup(params);
	creation.setup(params);
	continuation.setup(params);
	selection.setup(params);
	crossover.setup(params);
	mutation.setup(params);
	training.setup(params);
	replacement.setup(params);
}

void EvolutionTuning::display(std::ostream &o) const
{
	system.display(o);
	o << std::endl;

	solution.display(o);
	o << std::endl;

	optimization.display(o);
	o << std::endl;

	creation.display(o);
	o << std::endl;

	continuation.display(o);
	o << std::endl;

	selection.display(o);
	o << std::endl;

	crossover.display(o);
	o << std::endl;

	mutation.display(o);
	o << std::endl;

	training.display(o);
	o << std::endl;

	replacement.display(o);
}

std::ostream &operator<< (std::ostream &o, const Tuning &tuning)
{
	tuning.display(o);
	return o;
}
