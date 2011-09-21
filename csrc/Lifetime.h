#ifndef __LIFETIME_H__
#define __LIFETIME_H__

#include "common.h"

typedef std::pair<unsigned int, double> peak_t;
typedef std::list<peak_t > extrema_t;

class Lifetime
{
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
#ifndef FAKE_EVALUATION
	static const double Eatc = 0.7; /* eV, typically ranges from 0.3 up to 1.5 */
#else
	static const double Eatc = 0.08;
#endif

	/* Boltzmann constant [6] */
	static const double k = 8.61733248e-5; /* eV/K */

	/* Empirically determined constant */
	static const double Atc = 1;

	/* Shape parameter for the Weibull distribution */
	static const double beta = 2;

	public:

	static double predict(const matrix_t &temperature, double sampling_interval);

	private:

	static double calc_damage(const matrix_t &temperature);
	static void detect_peaks(const matrix_t &temperature,
		std::vector<extrema_t> &peaks);
	static void rainflow(const extrema_t &extrema, vector_t &amplitudes,
		vector_t &means);
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
