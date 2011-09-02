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
	chromosome_mutation_rate = 1.0;
	gene_mutation_rate = 0.05;

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

		if (name == "seed")
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
		else if (name == "crossover_rate")
			stream >> crossover_rate;
		else if (name == "crossover_points")
			stream >> crossover_points;

		/* Mutate */
		else if (name == "chromosome_mutation_rate")
			stream >> chromosome_mutation_rate;
		else if (name == "gene_mutation_rate")
			stream >> gene_mutation_rate;

		else if (name == "verbose")
			stream >> verbose;
		else if (name == "cache")
			stream >> cache;

		else if (name == "dump_evolution")
			stream >> dump_evolution;

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
		<< "  Chromosome mutation: " << tunning.chromosome_mutation_rate << std::endl
		<< "  Gene mutation rate:  " << tunning.gene_mutation_rate << std::endl;
}
