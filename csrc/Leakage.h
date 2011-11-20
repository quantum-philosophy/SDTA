#ifndef __LEAKAGE_H__
#define __LEAKAGE_H__

#include "common.h"
#include "Processor.h"

class Leakage
{
	static const size_t max_iterations = 10;
	static const double tolerance = 0.01;

	public:

	Leakage()
	{
	}

	virtual inline size_t get_max_iterations() const
	{
		return max_iterations;
	}

	virtual inline double get_tolerance() const
	{
		return tolerance;
	}

	virtual inline double **setup(double **conductance,
		double ambient_temperature) const
	{
		return conductance;
	}

	virtual inline void inject(double temperature,
		const double *dynamic_power, double *total_power,
		size_t step_count = 1) const
	{
	}

	virtual inline void inject(const double *temperature,
		const double *dynamic_power, double *total_power,
		size_t step_count = 1) const
	{
	}

	virtual inline void finalize(const double *temperature,
		const double *dyamic_power, double *total_power,
		size_t step_count = 1) const
	{
	}
};

class ExponentialLeakage: public Leakage
{
	static const double A = 1.1432e-12;
	static const double B = 1.0126e-14;
	static const double alpha = 466.4029;
	static const double beta = -1224.74083;
	static const double gamma = 6.28153;
	static const double delta = 6.9094;

	/* How to calculate Is?
	 *
	 * Take a look at (all coefficients above are from this paper):
	 * "Temperature and Supply Voltage Aware Performance and Power
	 * Modeling at Microarchitecture Level" (July 2005)
	 *
	 * T = [ 100, 100, 80, 80, 60, 60 ] + 273.15
	 * V = [ 0.95, 1.05, 0.95, 1.05, 0.95, 1.05 ]
	 * Iavg = [ 23.44, 29.56, 19.44, 25.14, 16.00, 21.33 ] * 1e-6
	 * Is = mean(Iavg(i) / favg(T(i), V(i)))
	 *
	 * Where favg is the scaling factor.
	 */
	static const double Is = 995.7996;

	const size_t processor_count;
	std::vector<double> voltages;
	std::vector<unsigned long int> ngates;

	public:

	ExponentialLeakage(const processor_vector_t &processors) :
		processor_count(processors.size())
	{
		voltages.resize(processor_count);
		ngates.resize(processor_count);

		for (size_t i = 0; i < processor_count; i++) {
			voltages[i] = processors[i]->get_voltage();
			ngates[i] = processors[i]->get_ngate();
		}
	}

	static double calculate(const Processor *processor, double temperature)
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

		const double voltage = processor->get_voltage();
		const double ngate = processor->get_ngate();

		const double favg = A * temperature * temperature *
			exp((alpha * voltage + beta) / temperature) +
			B * exp(gamma * voltage + delta);

		return ngate * Is * favg * voltage;
	}

	void inject(double temperature,
		const double *dynamic_power, double *total_power,
		size_t step_count = 1) const
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

	void inject(const double *temperature,
		const double *dynamic_power, double *total_power,
		size_t step_count = 1) const
	{
		size_t i, j, k;
		double favg, voltage;
		unsigned long int ngate;

		for (i = 0, k = 0; i < step_count; i++) {
			for (j = 0; j < processor_count; j++, k++) {
				voltage = voltages[j];
				ngate = ngates[j];

				favg = A * temperature[k] * temperature[k] *
					exp((alpha * voltage + beta) / temperature[k]) +
					B * exp(gamma * voltage + delta);

				total_power[k] = dynamic_power[k] + ngate * Is * favg * voltage;
			}
		}
	}
};

class BasicLinearLeakage: public Leakage
{
	static const double T1 = 30 + 273.15;
	static const double T2 = 80 + 273.15;
	static const size_t N = 20;

	protected:

	const size_t processor_count;

	double *k;
	double *b;

	public:

	BasicLinearLeakage(const processor_vector_t &processors) :
		processor_count(processors.size())
	{
		k = new double[processor_count];
		b = new double[processor_count];

		double sxy, sx, sy, ssx, T, P;

		/* Least square fit */
		for (size_t i = 0; i < processor_count; i++) {
			sxy = sx = sy = ssx = 0;

			for (size_t j = 0; j < N; j++) {
				T = T1 + double(j) * (T2 - T1) / double(N);
				P = ExponentialLeakage::calculate(processors[i], T);

				sxy += T * P;
				sx += T;
				sy += P;
				ssx += T * T;
			}

			k[i] = (double(N) * sxy - sx * sy) / (double(N) * ssx - sx * sx);
			b[i] = (sy - k[i] * sx) / double(N);
		}
	}

	virtual ~BasicLinearLeakage()
	{
		delete k;
		delete b;
	}
};

class LinearLeakage: public BasicLinearLeakage
{
	double *e;

	public:

	LinearLeakage(const processor_vector_t &processors) :
		BasicLinearLeakage(processors)
	{
		e = new double[processor_count];
	}

	~LinearLeakage()
	{
		delete e;
	}

	inline size_t get_max_iterations() const
	{
		return 1;
	}

	inline double get_tolerance() const
	{
		return 0;
	}

	inline double **setup(double **conductance,
		double ambient_temperature) const
	{
		for (size_t i = 0; i < processor_count; i++) {
			conductance[i][i] -= k[i];
			e[i] = b[i] + k[i] * ambient_temperature;
		}

		return conductance;
	}

	void inject(double temperature,
		const double *dynamic_power, double *total_power, size_t step_count) const
	{
		size_t i, j, l;

		for (i = 0, l = 0; i < step_count; i++)
			for (j = 0; j < processor_count; j++, l++)
				total_power[l] = dynamic_power[l] + e[j];
	}

	void inject(const double *temperature,
		const double *dynamic_power, double *total_power, size_t step_count) const
	{
		size_t i, j, l;

		for (i = 0, l = 0; i < step_count; i++)
			for (j = 0; j < processor_count; j++, l++)
				total_power[l] = dynamic_power[l] + e[j];
	}

	inline void finalize(const double *temperature,
		const double *dynamic_power, double *total_power, size_t step_count) const
	{
		size_t i, j, l;

		for (i = 0, l = 0; i < step_count; i++)
			for (j = 0; j < processor_count; j++, l++)
				total_power[l] = dynamic_power[l] + k[j] * temperature[l] + b[j];
	}
};

/* Actually, just linear */
class PiecewiseLinearLeakage: public BasicLinearLeakage
{
	public:

	PiecewiseLinearLeakage(const processor_vector_t &processors) :
		BasicLinearLeakage(processors)
	{
	}

	void inject(double temperature,
		const double *dynamic_power, double *total_power, size_t step_count) const
	{
		size_t i, j, l;

		for (i = 0, l = 0; i < step_count; i++)
			for (j = 0; j < processor_count; j++, l++)
				total_power[l] = dynamic_power[l] +
					temperature * k[j] + b[j];
	}

	void inject(const double *temperature,
		const double *dynamic_power, double *total_power, size_t step_count) const
	{
		size_t i, j, l;

		for (i = 0, l = 0; i < step_count; i++)
			for (j = 0; j < processor_count; j++, l++)
				total_power[l] = dynamic_power[l] +
					temperature[l] * k[j] + b[j];
	}
};

#endif
