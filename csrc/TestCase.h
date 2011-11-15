#ifndef __TEST_CASE_H__
#define __TEST_CASE_H__

#include <stdexcept>
#include <vector>

#include "Tuning.h"
#include "Architecture.h"
#include "Graph.h"
#include "Hotspot.h"
#include "Schedule.h"
#include "Layout.h"
#include "Priority.h"
#include "ListScheduler.h"
#include "Helper.h"

class TestCase
{
	public:

	const std::string floorplan_config;
	const std::string hotspot_config;
	const SolutionTuning solution_tuning;

	Graph *graph;
	Architecture *architecture;
	BasicListScheduler *scheduler;
	Leakage *leakage;
	Hotspot *hotspot;

	mapping_t mapping;
	priority_t priority;
	Schedule schedule;

#ifdef MEASURE_TIME
	double preparation_time;
#endif

	TestCase(const std::string &_system, const std::string &_floorplan,
		const std::string &_hotspot, const SystemTuning &system_tuning,
		const SolutionTuning &_solution_tuning) :

		floorplan_config(_floorplan), hotspot_config(_hotspot),
		solution_tuning(_solution_tuning),

		graph(NULL), architecture(NULL), scheduler(NULL),
		leakage(NULL), hotspot(NULL)
	{
		system_t system(_system);

		if (system_tuning.power_scale != 1) {
			if (system_tuning.verbose)
				std::cout << "Scaling the power consumption." << std::endl;

			size_t processor_count = system.ceff.size();
			for (size_t i = 0; i < processor_count; i++) {
				size_t type_count = system.ceff[i].size();
				for (size_t j = 0; j < type_count; j++)
					system.ceff[i][j] = system_tuning.power_scale * system.ceff[i][j];
			}
		}

		if (system_tuning.time_scale != 1) {
			if (system_tuning.verbose)
				std::cout << "Scaling the execution time." << std::endl;

			size_t processor_count = system.nc.size();
			for (size_t i = 0; i < processor_count; i++) {
				size_t type_count = system.nc[i].size();
				for (size_t j = 0; j < type_count; j++)
					system.nc[i][j] = system_tuning.time_scale * system.nc[i][j];
			}
		}

		graph = new GraphBuilder(system.type, system.link);
		architecture = new ArchitectureBuilder(system.frequency,
			system.voltage, system.ngate, system.nc, system.ceff);

		/* In order to assign a reasonable deadline and perform
		 * initial measurements to compare with, we:
		 * - assign a dummy mapping,
		 * - compute a mobility-based priority,
		 * - and obtain a schedule with help of List Scheduler.
		 */

		mapping = system.mapping;
		priority = system.priority;
		double deadline = system.deadline;

		/* 1. Calculate a priority vector based on the task mobility.
		 *
		 * NOTE: If the mapping is given, we are fine, the task mobility
		 * will be calculated properly, but if we do not have mapping,
		 * we do not know the execution time of the tasks, so, to deal
		 * with the problem, we assume them to be equal.
		 */
		if (priority.empty())
			priority = Priority::mobile(*architecture, *graph, mapping);
		else if (system_tuning.verbose)
			std::cout << "Using external priority." << std::endl;

		/* 2. Create and assign an even mapping.
		 *
		 */
		if (mapping.empty())
			mapping = Layout::earliest(*architecture, *graph, priority);
		else if (system_tuning.verbose)
			std::cout << "Using external mapping." << std::endl;

		/* 3. Compute a schedule.
		 *
		 */
		scheduler = new DeterministicListScheduler(*architecture, *graph);
		schedule = scheduler->process(mapping, priority);

		/* 4. Assign a deadline.
		 *
		 */
		if (deadline == 0)
			deadline = system_tuning.deadline_ratio * schedule.get_duration();
		else if (system_tuning.verbose)
			std::cout << "Using external deadline." << std::endl;

		graph->set_deadline(deadline);

		if (system_tuning.reorder_tasks) {
			if (system_tuning.verbose)
				std::cout << "Reordering tasks." << std::endl;

			/* Reorder the tasks if requested.
			 *
			 */
			size_t task_count = graph->size();

			/* Reordering according to the priority vector.
			 * First, get a vector with increasing components by 1.
			 */
			order_t order(task_count);
			for (size_t i = 0; i < task_count; i++) order[i] = i;

			/* Reorder the vector according to the given priority */
			Helper::prioritize(order, priority);

			/* Now, we can reorder everything */
			graph->reorder(order);
			Helper::permute<pid_t>(mapping, order);
			Helper::permute<rank_t>(priority, order);
			schedule.reorder(order);

			/* Since we are reordering according to the priority,
			 * the priority vector should become 0, 1, ..., (N - 1).
			 */
#ifndef SHALLOW_CHECK
			for (size_t i = 0; i < task_count; i++)
				if (priority[i] != i)
					throw std::runtime_error("The reordering does not work properly.");
#endif
		}

		/* Leakage model */
		if (solution_tuning.leakage == "linear") {
			leakage = new LinearLeakage(architecture->get_processors());
		}
		else if (solution_tuning.leakage == "piecewise_linear") {
			leakage = new PiecewiseLinearLeakage(architecture->get_processors());
		}
		else if (solution_tuning.leakage == "exponential") {
			leakage = new ExponentialLeakage(architecture->get_processors());
		}
		else if (solution_tuning.leak())
			throw std::runtime_error("The leakage model is unknown.");

#ifdef MEASURE_TIME
		struct timespec begin, end;
		Time::measure(&begin);
#endif

		hotspot = create_hotspot(solution_tuning.method);

#ifdef MEASURE_TIME
		Time::measure(&end);
		preparation_time = Time::substract(&end, &begin);
#endif
	}

	~TestCase()
	{
		__DELETE(graph);
		__DELETE(architecture);
		__DELETE(scheduler);
		__DELETE(leakage);
		__DELETE(hotspot);
	}

	Hotspot *create_hotspot(const std::string &method)
	{
		/* Thermal model */
		if (method == "condensed_equation") {
			if (leakage)
				return new LeakageCondensedEquationHotspot(
					*architecture, *graph, floorplan_config, hotspot_config,
					solution_tuning.hotspot, *leakage);
			else
				return new CondensedEquationHotspot(
					*architecture, *graph, floorplan_config, hotspot_config,
					solution_tuning.hotspot);
		}
		else if (method == "fixed_condensed_equation") {
			if (leakage)
				return new LeakageFixedCondensedEquationHotspot(
					*architecture, *graph, floorplan_config, hotspot_config,
					solution_tuning.hotspot, *leakage);
			else
				return new FixedCondensedEquationHotspot(
					*architecture, *graph, floorplan_config, hotspot_config,
					solution_tuning.hotspot);
		}
		else if (method == "coarse_condensed_equation") {
			if (leakage)
				throw std::runtime_error("Not implemented.");

			return new CoarseCondensedEquationHotspot(
				*architecture, *graph, floorplan_config, hotspot_config,
				solution_tuning.hotspot);
		}
		else if (method == "transient_analytical") {
			if (leakage)
				throw std::runtime_error("Not implemented.");

			return new TransientAnalyticalHotspot(
				*architecture, *graph, floorplan_config, hotspot_config,
				solution_tuning.hotspot, solution_tuning.max_iterations,
				solution_tuning.tolerance, solution_tuning.warmup);
		}
		else if (method == "hotspot") {
			if (leakage)
				return new LeakageIterativeHotspot(
					*architecture, *graph, floorplan_config, hotspot_config,
					solution_tuning.hotspot, solution_tuning.max_iterations,
					solution_tuning.tolerance, solution_tuning.warmup,
					*leakage);
			else
				return new IterativeHotspot(
					*architecture, *graph, floorplan_config, hotspot_config,
					solution_tuning.hotspot, solution_tuning.max_iterations,
					solution_tuning.tolerance, solution_tuning.warmup);
		}
		else if (method == "steady_state") {
			if (leakage)
				return new LeakageSteadyStateHotspot(
					*architecture, *graph, floorplan_config, hotspot_config,
					solution_tuning.hotspot, *leakage);
			else
				return new SteadyStateHotspot(
					*architecture, *graph, floorplan_config, hotspot_config,
					solution_tuning.hotspot);
		}
		else if (method == "precise_steady_state") {
			if (leakage)
				return new LeakagePreciseSteadyStateHotspot(
					*architecture, *graph, floorplan_config, hotspot_config,
					solution_tuning.hotspot, *leakage);
			else
				return new PreciseSteadyStateHotspot(
					*architecture, *graph, floorplan_config, hotspot_config,
					solution_tuning.hotspot);
		}
		else throw std::runtime_error("The solution method is unknown.");
	}
};

#endif
