#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Graph.h"
#include "Task.h"
#include "GeneticListScheduler.h"

void GLSTunning::defaults()
{
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
	crossover_points = 2;

	/* Mutate */
	mutation_rate = 0.05;
	mutation_points = 2;

	verbose = false;
	cache = true;
}

GLSTunning::GLSTunning(const char *filename)
{
	defaults();

	std::ifstream file(filename);
	file.exceptions(std::fstream::failbit | std::fstream::badbit);

	if (!file.is_open())
		throw std::runtime_error("Cannot open the tunning file.");

	std::string line, name;
	double value;

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
		stream >> value;

		if (name == "seed")
			seed = value;

		else if (name == "uniform_ratio")
			uniform_ratio = value;
		else if (name == "population_size")
			population_size = value;

		/* Continue */
		else if (name == "min_generations")
			min_generations = value;
		else if (name == "max_generations")
			max_generations = value;
		else if (name == "stall_generations")
			stall_generations = value;

		/* Select */
		else if (name == "elitism_rate")
			elitism_rate = value;
		else if (name == "tournament_size")
			tournament_size = value;

		/* Crossover */
		else if (name == "crossover_rate")
			crossover_rate = value;
		else if (name == "crossover_points")
			crossover_points = value;

		/* Mutate */
		else if (name == "mutation_rate")
			mutation_rate = value;
		else if (name == "mutation_points")
			mutation_points = value;

		else if (name == "verbose")
			verbose = value;
		else if (name == "cache")
			cache = value;

		else
			throw std::runtime_error("An unknown tunning parameter.");
	}
}

std::ostream &operator<< (std::ostream &o, const GLSTunning &tunning)
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Tunning:" << std::endl

		<< std::setprecision(0)
		<< "  Seed:                " << tunning.seed << std::endl

		/* Create */
		<< std::setprecision(2)
		<< "  Uniform ratio:       " << tunning.uniform_ratio << std::endl
		<< std::setprecision(0)
		<< "  Population size:     " << tunning.population_size << std::endl

		/* Continue */
		<< "  Minimum generations: " << tunning.min_generations << std::endl
		<< "  Maximum generations: " << tunning.max_generations << std::endl
		<< "  Stall generations:   " << tunning.stall_generations << std::endl

		/* Select */
		<< std::setprecision(2)
		<< "  Elitism rate:        " << tunning.elitism_rate << std::endl
		<< std::setprecision(0)
		<< "  Tournament size:     " << tunning.tournament_size << std::endl

		/* Crossover */
		<< std::setprecision(2)
		<< "  Crossover rate:      " << tunning.crossover_rate << std::endl
		<< std::setprecision(0)
		<< "  Crossover points:    " << tunning.crossover_points << std::endl

		/* Mutate */
		<< std::setprecision(2)
		<< "  Mutation rate:       " << tunning.mutation_rate << std::endl
		<< std::setprecision(0)
		<< "  Mutation points:     " << tunning.mutation_points << std::endl;
}

std::ostream &operator<< (std::ostream &o, const GLSStats &stats)
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Stats:" << std::endl

		<< std::setprecision(0)
		<< "  Generations:         " << stats.generations << std::endl
		<< "  Evaluations:         " << stats.evaluations << std::endl
		<< "  Cache hits:          " << stats.cache_hits << std::endl
		<< "  Deadline misses:     " << stats.deadline_misses << std::endl

		<< std::setiosflags(std::ios::scientific)
		<< std::setprecision(2)
		<< "  Best priority:       " << print_t<rank_t>(stats.priority) << std::endl
		<< std::setiosflags(std::ios::fixed)
		<< std::setprecision(0)
		<< "  Best schedule:       " << print_t<tid_t>(stats.schedule) << std::endl
		<< std::setprecision(2)
		<< "  Best fitness:        " << stats.fitness << std::endl;
}
