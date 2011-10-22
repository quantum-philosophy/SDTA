#ifndef __HOTSPOT_H__
#define __HOTSPOT_H__

extern "C" {
#include <hotspot/util.h>
#include <hotspot/flp.h>
#include <hotspot/temperature.h>
#include <hotspot/temperature_block.h>
}

#define __ALLOC(size) (double *)malloc(sizeof(double) * size)

#define __FREE(some) \
	do { \
		if (some) free(some); \
		some = NULL; \
	} while(0)

#include "Leakage.h"
#include "DynamicPower.h"

class Hotspot
{
	protected:

	size_t node_count;
	size_t processor_count;

	double sampling_interval;
	double ambient_temperature;

	thermal_config_t config;
	flp_t *floorplan;
	RC_model_t *model;

	public:

	Hotspot(const std::string &floorplan_filename,
		const std::string &config_filename,
		const std::string &config_line = std::string());
	virtual ~Hotspot();

	/* Without leakage */
	virtual void solve(const matrix_t &power, matrix_t &temperature)
	{
		throw std::runtime_error("Not implemented.");
	}

	/* With leakage */
	virtual void solve(const matrix_t &power, matrix_t &temperature,
		matrix_t &total_power)
	{
		throw std::runtime_error("Not implemented.");
	}

	/* With and without leakage from a schedule */
	virtual void solve(const Schedule &schedule, matrix_t &temperature,
		matrix_t &power)
	{
		throw std::runtime_error("Not implemented.");
	}

	inline double get_sampling_interval() const
	{
		return sampling_interval;
	}

	void get_conductance(matrix_t &conductance) const;
	void get_capacitance(vector_t &capacitance) const;
	void get_inversed_capacitance(vector_t &inversed_capacitance) const;
};

class BasicCondensedEquationHotspot: public Hotspot
{
	void *condensed_equation;

	public:

	BasicCondensedEquationHotspot(
		const std::string &floorplan_filename,
		const std::string &config_filename,
		const std::string &config_line = std::string());
	~BasicCondensedEquationHotspot();

	void solve(const matrix_t &power, matrix_t &temperature);
	void solve(const matrix_t &power, matrix_t &temperature, matrix_t &total_power);
};

class CondensedEquationHotspot: public BasicCondensedEquationHotspot
{
	const DynamicPower dynamic_power;

	public:

	CondensedEquationHotspot(
		const Architecture &architecture, const Graph &graph,
		const std::string &floorplan, const std::string &config,
		const std::string &config_line = std::string());

	void solve(const Schedule &schedule, matrix_t &temperature, matrix_t &power);
};

class BasicCondensedEquationLeakageHotspot: public Hotspot
{
	void *condensed_equation;

	public:

	BasicCondensedEquationLeakageHotspot(
		const Architecture &architecture,
		const std::string &floorplan, const std::string &config,
		const std::string &config_line = std::string());
	~BasicCondensedEquationLeakageHotspot();

	void solve(const matrix_t &power, matrix_t &temperature, matrix_t &total_power);
};

class CondensedEquationLeakageHotspot: public BasicCondensedEquationLeakageHotspot
{
	const DynamicPower dynamic_power;

	public:

	CondensedEquationLeakageHotspot(
		const Architecture &architecture, const Graph &graph,
		const std::string &floorplan, const std::string &config,
		const std::string &config_line = std::string());

	void solve(const Schedule &schedule, matrix_t &temperature, matrix_t &power);
};

typedef std::vector<int> SlotTrace;

class Slot
{
	const size_t width;

	Slot *blank;
	std::vector<Slot *> children;

	double *data;

	public:

	Slot(size_t _width) :
		width(_width), blank(NULL), children(_width, NULL), data(NULL) {}

	~Slot()
	{
		__DELETE(blank);

		size_t width = children.size();

		for (size_t i = 0; i < width; i++)
			__DELETE(children[i]);

		__FREE(data);
	}

	inline Slot *find(const SlotTrace &trace)
	{
		Slot *current = this;
		size_t i, size = trace.size();
		int id;

		for (i = 0; i < size; i++) {
			id = trace[i];

			if (id < 0) {
				if (!current->blank)
					current->blank = new Slot(width);

				current = current->blank;
			}
			else {
				if (!current->children[id])
					current->children[id] = new Slot(width);

				current = current->children[id];
			}
		}

		return current;
	}

	inline void store(double *data)
	{
#ifndef SHALLOW_CHECK
		if (this->data)
			throw std::runtime_error("The slot already has some data.");
#endif
		this->data = data;
	}

	inline double *get_data()
	{
		return data;
	}
};

class BasicSteadyStateHotspot: public Hotspot
{
	protected:

	const processor_vector_t &processors;
	const double deadline;
	const size_t step_count;

	private:

	Slot *storage;
	size_t type_count;

	std::vector<unsigned int> types;

	public:

	BasicSteadyStateHotspot(
		const Architecture &architecture, const Graph &graph,
		const std::string &floorplan, const std::string &config,
		const std::string &config_line = std::string());
	~BasicSteadyStateHotspot();

	void solve(const Schedule &schedule, matrix_t &temperature, matrix_t &power);

	protected:

	virtual double *compute(const SlotTrace &trace) const = 0;
	const double *get(const SlotTrace &trace);
};

class SteadyStateHotspot: public BasicSteadyStateHotspot
{
	public:

	SteadyStateHotspot(
		const Architecture &architecture, const Graph &graph,
		const std::string &floorplan, const std::string &config,
		const std::string &config_line = std::string()) :

		BasicSteadyStateHotspot(architecture, graph, floorplan,
			config, config_line) {}

	protected:

	double *compute(const SlotTrace &trace) const;
};

class SteadyStateLeakageHotspot: public BasicSteadyStateHotspot
{
	static const double tol = 0.01;
	static const size_t maxit = 10;

	LinearLeakage leakage;

	public:

	SteadyStateLeakageHotspot(
		const Architecture &architecture, const Graph &graph,
		const std::string &floorplan, const std::string &config,
		const std::string &config_line = std::string());

	protected:

	double *compute(const SlotTrace &trace) const;
};

class PreciseSteadyStateHotspot: public Hotspot
{
	const DynamicPower dynamic_power;

	public:

	PreciseSteadyStateHotspot(
		const Architecture &architecture, const Graph &graph,
		const std::string &floorplan, const std::string &config,
		const std::string &config_line);

	void solve(const Schedule &schedule, matrix_t &temperature, matrix_t &power);
	void solve(const matrix_t &power, matrix_t &temperature);
};

class IterativeHotspot: public Hotspot
{
	const size_t max_iterations;
	const double tolerance;

	public:

	IterativeHotspot(const std::string &floorplan,
		const std::string &config, const std::string &config_line,
		size_t _max_iterations, double _tolerance);

	size_t verify(const matrix_t &power, const matrix_t &reference_temperature,
		matrix_t &temperature);

	private:

	size_t solve(double *extended_power, const double *reference_temperature,
		double *temperature, size_t step_count);
};

struct Event
{
	int pid;
	int id;
	double time;

	Event(int _pid = -1, int _id = -1, double _time = 0) :
		pid(_pid), id(_id), time(_time) {}
};

class EventQueue
{
	const double deadline;

	size_t position;
	size_t length;

	std::vector<Event> events;

	public:

	EventQueue(const Schedule &schedule, double _deadline);

	inline bool next(Event &event)
	{
		if (position < length) {
			event = events[position++];
			return true;
		}
		return false;
	}

	inline static bool compare_events(const Event &one, const Event &another)
	{
		return one.time < another.time;
	}
};

#endif
