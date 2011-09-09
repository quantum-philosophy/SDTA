#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Graph.h"
#include "Task.h"
#include "GeneticListScheduler.h"

void GLSTuning::defaults()
{
	multiobjective = false;

	/* Randomness */
	seed = 0;

	/* Create */
	uniform_ratio = 0.5;
	population_size = 25;

	/* Continue */
	min_generations = 0;
	max_generations = 100;
	stall_generations = 20;

	/* Select */
	elitism_rate = 0.5;
	tournament_size = 3;

	/* Crossover */
	crossover_min_rate = 0.5;
	crossover_scale = 1;
	crossover_exponent = 0;
	crossover_points = 2;

	/* Mutate */
	mutation_min_rate = 0.01;
	mutation_scale = 1;
	mutation_exponent = -0.05;

	verbose = false;
	cache = true;
	reorder_tasks = false;
}

GLSTuning::GLSTuning(const char *filename)
{
	defaults();

	std::ifstream file(filename);
	file.exceptions(std::fstream::failbit | std::fstream::badbit);

	if (!file.is_open())
		throw std::runtime_error("Cannot open the tuning file.");

	std::string line, name;

	while (true) {
		try {
			std::getline(file, line);
		}
		catch (...) {
			break;
		}

		/* Skip empty lines and comments */
		if (line.empty() || line[0] == '#') continue;

		std::stringstream stream(line);
		stream.exceptions(std::ios::failbit | std::ios::badbit);

		stream >> name;

		if (name == "multiobjective")
			stream >> multiobjective;

		else if (name == "seed")
			stream >> seed;

		else if (name == "uniform_ratio")
			stream >> uniform_ratio;
		else if (name == "population_size")
			stream >> population_size;

		/* Continue */
		else if (name == "min_generations")
			stream >> min_generations;
		else if (name == "max_generations")
			stream >> max_generations;
		else if (name == "stall_generations")
			stream >> stall_generations;

		/* Select */
		else if (name == "elitism_rate")
			stream >> elitism_rate;
		else if (name == "tournament_size")
			stream >> tournament_size;

		/* Crossover */
		else if (name == "crossover_min_rate")
			stream >> crossover_min_rate;
		else if (name == "crossover_scale")
			stream >> crossover_scale;
		else if (name == "crossover_exponent")
			stream >> crossover_exponent;
		else if (name == "crossover_points")
			stream >> crossover_points;

		/* Mutate */
		else if (name == "mutation_min_rate")
			stream >> mutation_min_rate;
		else if (name == "mutation_scale")
			stream >> mutation_scale;
		else if (name == "mutation_exponent")
			stream >> mutation_exponent;

		else if (name == "verbose")
			stream >> verbose;
		else if (name == "cache")
			stream >> cache;
		else if (name == "reorder_tasks")
			stream >> reorder_tasks;

		else if (name == "dump_evolution")
			stream >> dump_evolution;

		else
			throw std::runtime_error("An unknown tuning parameter.");
	}
}

std::ostream &operator<< (std::ostream &o, const GLSTuning &tuning)
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Tuning:" << std::endl

		<< "  Multi-objective:         " << tuning.multiobjective << std::endl

		<< std::setprecision(0)
		<< "  Seed:                    " << tuning.seed << std::endl

		/* Create */
		<< std::setprecision(2)
		<< "  Uniform ratio:           " << tuning.uniform_ratio << std::endl
		<< std::setprecision(0)
		<< "  Population size:         " << tuning.population_size << std::endl

		/* Continue */
		<< "  Minimum generations:     " << tuning.min_generations << std::endl
		<< "  Maximum generations:     " << tuning.max_generations << std::endl
		<< "  Stall generations:       " << tuning.stall_generations << std::endl

		/* Select */
		<< std::setprecision(2)
		<< "  Elitism rate:            " << tuning.elitism_rate << std::endl
		<< std::setprecision(0)
		<< "  Tournament size:         " << tuning.tournament_size << std::endl

		/* Crossover */
		<< std::setprecision(2)
		<< "  Crossover minimal rate:  " << tuning.crossover_min_rate << std::endl
		<< "  Crossover scale:         " << tuning.crossover_scale << std::endl
		<< "  Crossover exponent:      " << tuning.crossover_exponent << std::endl
		<< std::setprecision(0)
		<< "  Crossover points:        " << tuning.crossover_points << std::endl

		/* Mutate */
		<< std::setprecision(2)
		<< "  Mutation minimal rate:   " << tuning.mutation_min_rate << std::endl
		<< "  Mutation scale:          " << tuning.mutation_scale << std::endl
		<< "  Mutation exponent:       " << tuning.mutation_exponent << std::endl;
}
