#include "Leakage.h"
#include "Processor.h"

Leakage::Leakage(const processor_vector_t &processors) :
	processor_count(processors.size())
{
	voltages.resize(processor_count);
	ngates.resize(processor_count);

	for (size_t i = 0; i < processor_count; i++) {
		voltages[i] = processors[i]->get_voltage();
		ngates[i] = processors[i]->get_ngate();
	}
}

void Leakage::inject(
	size_t step_count, const double *dynamic_power,
	const double *temperature, double *total_power) const
{
	/* Pleak = Ngate * Iavg * Vdd
	 * Iavg(T, Vdd) = Is(T0, V0) * favg(T, Vdd)
	 *
	 * Where the scaling factor:
	 * f(T, Vdd) = A * T^2 * e^((alpha * Vdd + beta)/T) +
	 *   B * e^(gamma * Vdd + delta)
	 *
	 * From:
	 * "Temperature and Supply Voltage Aware Performance and
	 * Power Modeling at Microarchitecture Level"
	 */

	size_t i, j, k;
	double temp, favg, voltage;
	unsigned long int ngate;

	for (i = 0, k = 0; i < step_count; i++) {
		for (j = 0; j < processor_count; j++, k++) {
			temp = temperature[k];
			voltage = voltages[j];
			ngate = ngates[j];

			favg = A * temp * temp *
				exp((alpha * voltage + beta) / temp) +
				B * exp(gamma * voltage + delta);

			total_power[k] = dynamic_power[k] + ngate * Is * favg * voltage;
		}
	}
}

void Leakage::inject(
	size_t step_count, const double *dynamic_power,
	double temperature, double *total_power) const
{
	size_t i, j, k;
	double favg, voltage;
	unsigned long int ngate;

	for (i = 0, k = 0; i < step_count; i++) {
		for (j = 0; j < processor_count; j++, k++) {
			voltage = voltages[j];
			ngate = ngates[j];

			favg = A * temperature * temperature *
				exp((alpha * voltage + beta) / temperature) +
				B * exp(gamma * voltage + delta);

			total_power[k] = dynamic_power[k] + ngate * Is * favg * voltage;
		}
	}
}

LinearLeakage::LinearLeakage(const processor_vector_t &processors) :
	Leakage(processors) {}

void LinearLeakage::inject(
	size_t step_count, const double *dynamic_power,
	const double *temperature, double *total_power) const
{
	size_t i, j, k;

	for (i = 0, k = 0; i < step_count; i++)
		for (j = 0; j < processor_count; j++, k++)
			total_power[k] = dynamic_power[k] +
				temperature[k] * this->k + this->b;
}

void LinearLeakage::inject(
	size_t step_count, const double *dynamic_power,
	double temperature, double *total_power) const
{
	size_t i, j, k;

	for (i = 0, k = 0; i < step_count; i++)
		for (j = 0; j < processor_count; j++, k++)
			total_power[k] = dynamic_power[k] +
				temperature * this->k + this->b;
}
