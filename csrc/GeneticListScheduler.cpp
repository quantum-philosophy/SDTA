#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Graph.h"
#include "Task.h"
#include "GeneticListScheduler.h"

/******************************************************************************/
/* GLSTuning                                                                  */
/******************************************************************************/

void GLSTuning::defaults()
{
	/* Preparation */
	repeat = -1;
	deadline_ratio = 1.1;
	reorder_tasks = false;

	/* Method */
	multiobjective = false;

	/* Randomness */
	seed = -1;

	/* Create */
	uniform_ratio = 0.5;
	population_size = 25;

	/* Continue */
	min_generations = 0;
	max_generations = 100;
	stall_generations = 20;

	/* Select */
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

	/* Evolve */
	elitism_rate = 0.5;

	/* Speed up */
	cache = true;

	/* Output */
	verbose = false;
	dump_evolution = std::string();
}

GLSTuning::GLSTuning(const std::string &filename)
{
	defaults();

	if (filename.empty()) return;

	std::ifstream file(filename.c_str());

	if (!file.is_open())
		throw std::runtime_error("Cannot open the tuning file.");

	update(file);
}

void GLSTuning::update(std::istream &stream)
{
	stream.exceptions(std::ios::failbit | std::ios::badbit);

	std::string line, name;

	while (true) {
		try {
			std::getline(stream, line);
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
			stream >> min_generations;
		else if (name == "max_generations")
			stream >> max_generations;
		else if (name == "stall_generations")
			stream >> stall_generations;

		/* Select */
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

		/* Evolve */
		else if (name == "elitism_rate")
			stream >> elitism_rate;

		/* Speed up */
		else if (name == "cache")
			stream >> cache;

		/* Output */
		else if (name == "verbose")
			stream >> verbose;
		else if (name == "dump_evolution")
			stream >> dump_evolution;

		else
			throw std::runtime_error("An unknown tuning parameter.");
	}
}

void GLSTuning::display(std::ostream &o) const
{
	o
		<< std::setiosflags(std::ios::fixed)

		<< "Tuning:" << std::endl

		/* Prepare */
		<< "  Repeat:                  " << repeat << std::endl
		<< std::setprecision(2)
		<< "  Deadline ratio:          " << deadline_ratio << std::endl
		<< "  Reorder tasks:           " << reorder_tasks << std::endl

		/* Target */
		<< "  Multi-objective:         " << multiobjective << std::endl

		/* Randomize */
		<< std::setprecision(0)
		<< "  Seed:                    " << seed << std::endl

		/* Create */
		<< std::setprecision(2)
		<< "  Uniform ratio:           " << uniform_ratio << std::endl
		<< std::setprecision(0)
		<< "  Population size:         " << population_size << std::endl

		/* Continue */
		<< "  Minimum generations:     " << min_generations << std::endl
		<< "  Maximum generations:     " << max_generations << std::endl
		<< "  Stall generations:       " << stall_generations << std::endl

		/* Select */
		<< std::setprecision(0)
		<< "  Tournament size:         " << tournament_size << std::endl

		/* Crossover */
		<< std::setprecision(2)
		<< "  Crossover minimal rate:  " << crossover_min_rate << std::endl
		<< "  Crossover scale:         " << crossover_scale << std::endl
		<< "  Crossover exponent:      " << crossover_exponent << std::endl
		<< std::setprecision(0)
		<< "  Crossover points:        " << crossover_points << std::endl

		/* Mutate */
		<< std::setprecision(2)
		<< "  Mutation minimal rate:   " << mutation_min_rate << std::endl
		<< "  Mutation scale:          " << mutation_scale << std::endl
		<< "  Mutation exponent:       " << mutation_exponent << std::endl

		/* Evolve */
		<< std::setprecision(2)
		<< "  Elitism rate:            " << elitism_rate << std::endl

		/* Speed up */
		<< "  Use caching:             " << cache << std::endl

		/* Output */
		<< "  Verbose:                 " << verbose << std::endl
		<< "  Dump evolution:          " << dump_evolution << std::endl;
}

std::ostream &operator<< (std::ostream &o, const GLSTuning &tuning)
{
	tuning.display(o);
	return o;
}

std::ostream &operator<< (std::ostream &o, const GeneticListSchedulerStats &stats)
{
	stats.display(o);
	return o;
}
