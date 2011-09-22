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
		else if (name == "max_generations")
			stream >> max_generations;
		else if (name == "stall_generations")
			stream >> stall_generations;

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
		else if (name == "elitism_rate")
			stream >> elitism_rate;

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
		<< "  Repeat:                  " << repeat << std::endl
		<< std::setprecision(3)
		<< "  Deadline ratio:          " << deadline_ratio << std::endl
		<< "  Reorder tasks:           " << reorder_tasks << std::endl
		<< "  Consider mapping:        " << include_mapping << std::endl
		<< std::endl

		/* Target */
		<< "  Multi-objective:         " << multiobjective << std::endl
		<< std::endl

		/* Randomize */
		<< std::setprecision(0)
		<< "  Seed:                    " << seed << std::endl
		<< std::endl

		/* Create */
		<< std::setprecision(3)
		<< "  Uniform ratio:           " << uniform_ratio << std::endl
		<< std::setprecision(0)
		<< "  Population size:         " << population_size << std::endl
		<< std::endl

		/* Continue */
		<< "  Maximum generations:     " << max_generations << std::endl
		<< "  Stall generations:       " << stall_generations << std::endl
		<< std::endl

		/* Select */
		<< "  Selection:               " << selection.method << std::endl
		<< std::setprecision(2)
		<< "  Selection ratio:         " << selection.ratio << std::endl
		<< std::setprecision(0)
		<< "  Tournament size:         " << selection.tournament_size << std::endl
		<< std::endl

		/* Crossover */
		<< "  Crossover:               " << crossover.method << std::endl
		<< std::setprecision(3)
		<< "  Crossover minimal rate:  " << crossover.min_rate << std::endl
		<< "  Crossover scale:         " << crossover.scale << std::endl
		<< "  Crossover exponent:      " << crossover.exponent << std::endl
		<< std::setprecision(0)
		<< "  Crossover points:        " << crossover.points << std::endl
		<< std::endl

		/* Mutate */
		<< "  Mutation:                " << mutation.method << std::endl
		<< std::setprecision(3)
		<< "  Mutation minimal rate:   " << mutation.min_rate << std::endl
		<< "  Mutation scale:          " << mutation.scale << std::endl
		<< "  Mutation exponent:       " << mutation.exponent << std::endl
		<< std::endl

		/* Train */
		<< "  Training:                " << training.method << std::endl
		<< std::setprecision(3)
		<< "  Training minimal rate:   " << training.min_rate << std::endl
		<< "  Training scale:          " << training.scale << std::endl
		<< "  Training exponent:       " << training.exponent << std::endl
		<< std::setprecision(0)
		<< "  Maximum lessons:         " << training.max_lessons << std::endl
		<< "  Stall lessons:           " << training.stall_lessons << std::endl
		<< std::endl

		/* Evolve */
		<< std::setprecision(3)
		<< "  Elitism rate:            " << elitism_rate << std::endl
		<< std::endl

		/* Output */
		<< "  Verbose:                 " << verbose << std::endl
		<< "  Dump evolution:          " << dump_evolution << std::endl;
}

std::ostream &operator<< (std::ostream &o, const EvolutionTuning &tuning)
{
	tuning.display(o);

	return o;
}
