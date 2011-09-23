#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "EvolutionTuning.h"

void EvolutionTuning::update(const std::string &filename)
{
	std::ifstream file(filename.c_str());

	if (!file.is_open())
		throw std::runtime_error("Cannot open the tuning file.");

	update(file);
}

void EvolutionTuning::update(std::istream &main_stream)
{
	main_stream.exceptions(std::ios::failbit | std::ios::badbit);

	std::string line, name;

	while (true) {
		try {
			std::getline(main_stream, line);
		}
		catch (...) {
			break;
		}

		/* Skip empty lines and comments */
		if (line.empty() || line[0] == '#') continue;

		std::stringstream stream(line);
		stream.exceptions(std::ios::failbit | std::ios::badbit);

		stream >> name;

		/* Prepare */
		if (name == "repeat")
			stream >> repeat;
		else if (name == "deadline_ratio")
			stream >> deadline_ratio;
		else if (name == "reorder_tasks")
			stream >> reorder_tasks;
		else if (name == "include_mapping")
			stream >> include_mapping;

		/* Target */
		else if (name == "multiobjective")
			stream >> multiobjective;

		/* Randomize */
		else if (name == "seed")
			stream >> seed;

		/* Create */
		else if (name == "uniform_ratio")
			stream >> uniform_ratio;
		else if (name == "population_size")
			stream >> population_size;

		/* Continue */
		else if (name == "min_generations")
			stream >> continuation.min_generations;
		else if (name == "max_generations")
			stream >> continuation.max_generations;
		else if (name == "stall_generations")
			stream >> continuation.stall_generations;

		/* Select */
		else if (name == "selection")
			stream >> selection.method;
		else if (name == "selection_ratio")
			stream >> selection.ratio;
		else if (name == "tournament_size")
			stream >> selection.tournament_size;

		/* Crossover */
		else if (name == "crossover")
			stream >> crossover.method;
		else if (name == "crossover_min_rate")
			stream >> crossover.min_rate;
		else if (name == "crossover_scale")
			stream >> crossover.scale;
		else if (name == "crossover_exponent")
			stream >> crossover.exponent;
		else if (name == "crossover_points")
			stream >> crossover.points;

		/* Mutate */
		else if (name == "mutation")
			stream >> mutation.method;
		else if (name == "mutation_min_rate")
			stream >> mutation.min_rate;
		else if (name == "mutation_scale")
			stream >> mutation.scale;
		else if (name == "mutation_exponent")
			stream >> mutation.exponent;

		/* Train */
		else if (name == "training")
			stream >> training.method;
		else if (name == "training_min_rate")
			stream >> training.min_rate;
		else if (name == "training_scale")
			stream >> training.scale;
		else if (name == "training_exponent")
			stream >> training.exponent;
		else if (name == "max_lessons")
			stream >> training.max_lessons;
		else if (name == "stall_lessons")
			stream >> training.stall_lessons;

		/* Evolve */
		else if (name == "replacement")
			stream >> replacement.method;
		else if (name == "elitism_rate")
			stream >> replacement.elitism_rate;

		/* Output */
		else if (name == "verbose")
			stream >> verbose;
		else if (name == "dump_evolution")
			stream >> dump_evolution;

		else
			throw std::runtime_error("An unknown tuning parameter.");
	}
}

void EvolutionTuning::display(std::ostream &o) const
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Tuning:" << std::endl

		/* Prepare */
		<< "  Repeat:                 " << repeat << std::endl
		<< std::setprecision(3)
		<< "  Deadline ratio:         " << deadline_ratio << std::endl
		<< "  Reorder tasks:          " << reorder_tasks << std::endl
		<< "  Consider mapping:       " << include_mapping << std::endl
		<< std::endl

		/* Target */
		<< "  Multi-objective:        " << multiobjective << std::endl
		<< std::endl

		/* Randomize */
		<< std::setprecision(0)
		<< "  Seed:                   " << seed << std::endl
		<< std::endl

		/* Create */
		<< std::setprecision(3)
		<< "  Uniform ratio:          " << uniform_ratio << std::endl
		<< std::setprecision(0)
		<< "  Population size:        " << population_size << std::endl
		<< std::endl

		/* Continue */
		<< "  Continuation:" << std::endl
		<< "    Minimal generations:  " << continuation.min_generations << std::endl
		<< "    Maximum generations:  " << continuation.max_generations << std::endl
		<< "    Stall generations:    " << continuation.stall_generations << std::endl
		<< std::endl

		/* Select */
		<< "  Selection:              " << selection.method << std::endl
		<< std::setprecision(2)
		<< "    Ratio:                " << selection.ratio << std::endl
		<< std::setprecision(0)
		<< "    Tournament size:      " << selection.tournament_size << std::endl
		<< std::endl

		/* Crossover */
		<< "  Crossover:              " << crossover.method << std::endl
		<< std::setprecision(3)
		<< "    Minimal rate:         " << crossover.min_rate << std::endl
		<< "    Scale:                " << crossover.scale << std::endl
		<< "    Exponent:             " << crossover.exponent << std::endl
		<< std::setprecision(0)
		<< "    Points:               " << crossover.points << std::endl
		<< std::endl

		/* Mutate */
		<< "  Mutation:               " << mutation.method << std::endl
		<< std::setprecision(3)
		<< "    Minimal rate:         " << mutation.min_rate << std::endl
		<< "    Scale:                " << mutation.scale << std::endl
		<< "    Exponent:             " << mutation.exponent << std::endl
		<< std::endl

		/* Train */
		<< "  Training:               " << training.method << std::endl
		<< std::setprecision(3)
		<< "    Minimal rate:         " << training.min_rate << std::endl
		<< "    Scale:                " << training.scale << std::endl
		<< "    Exponent:             " << training.exponent << std::endl
		<< std::setprecision(0)
		<< "    Maximum lessons:      " << training.max_lessons << std::endl
		<< "    Stall lessons:        " << training.stall_lessons << std::endl
		<< std::endl

		/* Evolve */
		<< "  Replacement:          " << replacement.method << std::endl
		<< std::setprecision(2)
		<< "    Elitism rate:       " << replacement.elitism_rate << std::endl
		<< std::endl

		/* Output */
		<< "  Verbose:              " << verbose << std::endl
		<< "  Dump evolution:       " << dump_evolution << std::endl;
}

std::ostream &operator<< (std::ostream &o, const EvolutionTuning &tuning)
{
	tuning.display(o);

	return o;
}
