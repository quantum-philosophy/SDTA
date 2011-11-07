#ifndef __LIFETIME_H__
#define __LIFETIME_H__

#include "common.h"

typedef std::pair<unsigned int, double> peak_t;
typedef std::list<peak_t > extrema_t;

class Lifetime
{
	public:

	virtual double predict(const matrix_t &temperature,
		double sampling_interval) = 0;
};

#define MAX_EXTREMA 1000

class ThermalCyclingLifetime: public Lifetime
{
	protected:

    /* Peak threshold of local minima and maxima (for the cycle counting) */
    static const double delta = 2; /* K */

	/* Coffin-Manson equation [2]
	 * Nf = C0 * (dT - dT0)^(-q)
	 */

	/* Coffin-Manson exponent [2] */
	static const double q = 6; /* from 6 to 9 for brittle fracture */

	/* Portion of the temperature range in the elastic region [2] */
	static const double dT0 = 0;

	/* Coffin-Manson equation with the Arrhenius term [3]
	 * Ntc = Atc * (dT - dT0)^(-q) * exp(Eatc / (k * Tmax))
	 */

	/* Activation energy [4], [5] */
	static const double Eatc = 0.3; /* eV, typically ranges from 0.3 up to 1.5 */

	/* Boltzmann constant [6] */
	static const double k = 8.61733248e-5; /* eV/K */

	/* Empirically determined constant */
	static const double Atc = 1e10;

	/* Shape parameter for the Weibull distribution */
	static const double beta = 2;

	public:

	virtual double predict(const matrix_t &temperature, double sampling_interval);

	protected:

	double a[MAX_EXTREMA];
	double amplitudes[MAX_EXTREMA];
	double means[MAX_EXTREMA];
	double cycles[MAX_EXTREMA];

	void detect_peaks(const matrix_t &temperature,
		std::vector<extrema_t> &peaks) const;
	size_t rainflow(const extrema_t &extrema);
};

class CombinedThermalCyclingLifetime: public ThermalCyclingLifetime
{
	public:

	virtual double predict(const matrix_t &temperature, double sampling_interval);
};

/* % References:
 * [1] http://en.wikipedia.org/wiki/Fatigue_(material)
 * [2] Failure Mechanisms and Models for Semiconductor Devices
 * [3] System-Level Reliability Modeling for MPSoCs
 * [4] http://www.siliconfareast.com/activation-energy.htm
 * [5] http://rel.intersil.com/docs/rel/calculation_of_semiconductor_failure_rates.pdf
 * [6] http://en.wikipedia.org/wiki/Boltzmann_constant
 */

#endif
