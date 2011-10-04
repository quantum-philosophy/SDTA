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

class Hotspot
{
	protected:

	const processor_vector_t &processors;
	const size_t processor_count;

	const task_vector_t &tasks;
	const size_t task_count;

	const double deadline;

	size_t node_count;

	double sampling_interval;
	double ambient_temperature;

	thermal_config_t config;
	flp_t *floorplan;
	RC_model_t *model;

	public:

	Hotspot(const Architecture &architecture, const Graph &graph,
		const std::string &floorplan_filename, const std::string &config_filename);
	virtual ~Hotspot();

	virtual void solve(const Schedule &schedule, matrix_t &temperature,
		matrix_t &power) = 0;

	inline double get_sampling_interval() const
	{
		return sampling_interval;
	}
};

class HotspotWithDynamicPower: public Hotspot
{
	std::vector<unsigned int> types;

	public:

	HotspotWithDynamicPower(const Architecture &architecture, const Graph &graph,
		const std::string &floorplan, const std::string &config);

	protected:

	void compute_power(const Schedule &schedule, matrix_t &power) const;
};

class HotspotWithoutLeakage: public HotspotWithDynamicPower
{
	void *condensed_equation;

	public:

	HotspotWithoutLeakage(const Architecture &architecture, const Graph &graph,
		const std::string &floorplan, const std::string &config);
	~HotspotWithoutLeakage();

	void solve(const Schedule &schedule, matrix_t &temperature,
		matrix_t &power);
};

class HotspotWithLeakage: public HotspotWithDynamicPower
{
	void *condensed_equation;

	public:

	HotspotWithLeakage(const Architecture &architecture, const Graph &graph,
		const std::string &floorplan, const std::string &config);
	~HotspotWithLeakage();

	void solve(const Schedule &schedule, matrix_t &temperature,
		matrix_t &total_power);
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

class SteadyStateHotspot: public Hotspot
{
	size_t step_count;
	size_t type_count;

	std::vector<unsigned int> types;

	Slot *storage;

	public:

	SteadyStateHotspot(const Architecture &architecture, const Graph &graph,
		const std::string &floorplan, const std::string &config);
	~SteadyStateHotspot();

	void solve(const Schedule &schedule, matrix_t &temperature,
		matrix_t &power);

	protected:

	double *compute(const SlotTrace &trace) const;
	const double *get(const SlotTrace &trace);
};

#endif
