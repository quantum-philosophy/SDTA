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

	Graph *graph;
	Architecture *architecture;
	BasicListScheduler *scheduler;
	Hotspot *hotspot;

	mapping_t mapping;
	priority_t priority;
	Schedule schedule;

	TestCase(const std::string &_system, const std::string &_floorplan,
		const std::string &_hotspot, const SystemTuning &tuning) :

		graph(NULL), architecture(NULL), scheduler(NULL), hotspot(NULL)
	{
		system_t system(_system);

		if (tuning.power_scale != 1) {
			if (tuning.verbose)
				std::cout << "Scaling the power consumption." << std::endl;

			size_t processor_count = system.ceff.size();
			for (size_t i = 0; i < processor_count; i++) {
				size_t type_count = system.ceff[i].size();
				for (size_t j = 0; j < type_count; j++)
					system.ceff[i][j] = tuning.power_scale * system.ceff[i][j];
			}
		}

		if (tuning.time_scale != 1) {
			if (tuning.verbose)
				std::cout << "Scaling the execution time." << std::endl;

			size_t processor_count = system.nc.size();
			for (size_t i = 0; i < processor_count; i++) {
				size_t type_count = system.nc[i].size();
				for (size_t j = 0; j < type_count; j++)
					system.nc[i][j] = tuning.time_scale * system.nc[i][j];
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
		else if (tuning.verbose)
			std::cout << "Using external priority." << std::endl;

		/* 2. Create and assign an even mapping.
		 *
		 */
		if (mapping.empty())
			mapping = Layout::earliest(*architecture, *graph, priority);
		else if (tuning.verbose)
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
			deadline = tuning.deadline_ratio * schedule.get_duration();
		else if (tuning.verbose)
			std::cout << "Using external deadline." << std::endl;

		graph->set_deadline(deadline);

		if (tuning.reorder_tasks) {
			if (tuning.verbose)
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

		if (tuning.solution == "condensed_equation") {
			/* People want leakage! */
			if (tuning.leakage)
				hotspot = new CondensedEquationLeakageHotspot(
					*architecture, *graph, _floorplan, _hotspot, tuning.hotspot);
			else
				hotspot = new CondensedEquationHotspot(
					*architecture, *graph, _floorplan, _hotspot, tuning.hotspot);
		}
		else if (tuning.solution == "coarse_condensed_equation") {
			if (tuning.leakage)
				throw std::runtime_error("Not implemented yet.");
			hotspot = new CoarseCondensedEquationHotspot(
				*architecture, *graph, _floorplan, _hotspot, tuning.hotspot);
		}
		else if (tuning.solution == "hotspot") {
			if (tuning.leakage)
				throw std::runtime_error("Not implemented yet.");
			hotspot = new IterativeHotspot(
				_floorplan, _hotspot, tuning.hotspot,
				tuning.max_iterations, tuning.tolerance);
		}
		else if (tuning.solution == "steady_state") {
			if (tuning.leakage)
				hotspot = new SteadyStateLeakageHotspot(
					*architecture, *graph, _floorplan, _hotspot, tuning.hotspot);
			else
				hotspot = new SteadyStateHotspot(
					*architecture, *graph, _floorplan, _hotspot, tuning.hotspot);
		}
		else if (tuning.solution == "precise_steady_state") {
			if (tuning.leakage)
				throw std::runtime_error("Not implemented yet.");
			hotspot = new PreciseSteadyStateHotspot(
				*architecture, *graph, _floorplan, _hotspot, tuning.hotspot);
		}
		else throw std::runtime_error("The solution method is unknown.");
	}

	~TestCase()
	{
		__DELETE(graph);
		__DELETE(architecture);
		__DELETE(scheduler);
		__DELETE(hotspot);
	}
};

#endif
