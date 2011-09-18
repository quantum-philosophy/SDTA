#include <stdexcept>
#include <iomanip>

#include "Architecture.h"
#include "Processor.h"
#include "Task.h"

void Architecture::add_processor(Processor *processor)
{
	processors.push_back(processor);
	processor_count = processors.size();
	processor->id = processor_count - 1;
}

std::ostream &operator<< (std::ostream &o, const Architecture *architecture)
{
	o	<< "Architecture:" << std::endl
		<< "  Number of processors: " << architecture->processor_count << std::endl;

	o	<< "  "
		<< std::setw(4) << "id" << " ( "
		<< std::setw(8) << "freq" << " : "
		<< std::setw(8) << "volt" << " : "
		<< std::setw(8) << "types" << " )" << std::endl;

	for (pid_t id = 0; id < architecture->processor_count; id++)
		o << "  " << architecture->processors[id];

	return o;
}

ArchitectureBuilder::ArchitectureBuilder(const std::vector<double> &frequency,
	const std::vector<double> &voltage,
	const std::vector<unsigned long int> &ngate,
	const std::vector<std::vector<unsigned long int> > &nc,
	const std::vector<std::vector<double> > &ceff) : Architecture()
{
	Processor *processor;

	size_t processor_count = frequency.size();

	if (processor_count == 0)
		throw std::runtime_error("There are no processors specified.");

	if (processor_count != voltage.size())
		throw std::runtime_error("The voltage vector is not consistent.");

	if (processor_count != ngate.size())
		throw std::runtime_error("The ngate vector is not consistent.");

	if (processor_count != nc.size())
		throw std::runtime_error("The nc vector is not consistent.");

	size_t type_count = nc[0].size();

	for (size_t i = 1; i < processor_count; i++)
		if (type_count != nc[i].size())
			throw std::runtime_error("The nc vector is not consistent.");

	if (processor_count != ceff.size())
		throw std::runtime_error("The ceff vector is not consistent.");

	for (size_t i = 0; i < processor_count; i++)
		if (type_count != ceff[i].size())
			throw std::runtime_error("The ceff vector is not consistent.");

	for (pid_t id = 0; id < processor_count; id++) {
		processor = new Processor(frequency[id], voltage[id], ngate[id]);
		add_processor(processor);

		size_t type_count = nc[id].size();
		for (size_t i = 0; i < type_count; i++)
			processor->add_type(nc[id][i], ceff[id][i]);
	}
}

ArchitectureBuilder::~ArchitectureBuilder()
{
	size_t processor_count = processors.size();
	for (size_t i = 0; i < processor_count; i++)
		delete processors[i];
}
