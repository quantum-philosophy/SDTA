#include "Lifetime.h"
#include "Architecture.h"
#include "Graph.h"

double ThermalCyclingLifetime::predict(
	const matrix_t &temperature, double sampling_interval)
{
	const double Q = q; /* Some stupid joke */

	size_t processor_count = temperature.cols();
	size_t step_count = temperature.rows();

	/* Get extrema */
	std::vector<extrema_t> peaks;
	detect_peaks(temperature, peaks);

	double maximal_damage = 0;

	/* For each temperature curve */
	for (size_t i = 0; i < processor_count; i++) {
		size_t cycle_count = rainflow(peaks[i]);

		double damage = 0;

		for (size_t j = 0; j < cycle_count; j++) {
			double dT = 2 * amplitudes[j];

			/* Skip cycles that do not cause any damage */
			if (dT <= dT0) continue;

			/* Maximal temperatures during each cycle */
			double Tmax = means[j] + amplitudes[j];

			/* Number of cycles to failure for each stress level [3] */
			double N = Atc * pow(dT - dT0, -Q) * exp(Eatc / (k * Tmax));

			damage += cycles[j] / N;
		}

		if (damage > maximal_damage) maximal_damage = damage;
	}

	return (sampling_interval * step_count) / maximal_damage;
}

void ThermalCyclingLifetime::detect_peaks(
	const matrix_t &data, std::vector<extrema_t> &peaks) const
{
	size_t rows = data.rows();
	size_t cols = data.cols();

	size_t mxpos, mnpos, col, row, firstpos;
	double current, mx, mn;

	bool look_for_max, found;

	peaks.resize(cols);

	/* For each column */
	for (col = 0; col < cols; col++) {
		mx = mn = data[0][col];
		mxpos = mnpos = 0;

		look_for_max = false;

		peaks[col].clear();

		/* For each row */
		for (row = 1; row < rows; row++) {
			current = data[row][col];

			if (current > mx) {
				mx = current;
				mxpos = row;
			}
			if (current < mn) {
				mn = current;
				mnpos = row;
			}

			if (look_for_max) {
				if (current < (mx - delta)) {
					peaks[col].push_back(peak_t(mxpos, mx));
					mn = current;
					mnpos = row;
					look_for_max = false;
				}
			}
			else {
				if (current > (mn + delta)) {
					peaks[col].push_back(peak_t(mnpos, mn));
					mx = current;
					mxpos = row;
					look_for_max = true;
				}
			}
		}

		/* The first one is always a minimum, so if the last one is
		 * a maximum, we are fine.
		 */
		if (!look_for_max) continue;

		/* The last one was a minimum which means that either we have
		 * missed a maximum in the beginning, or the first minimum is
		 * larger then the last one and should be eliminated.
		 */

		firstpos = peaks[col].begin()->first;
		found = false;

		for (row = 0; row < firstpos; row++) {
			current = data[row][col];

			if (current > mx) {
				mx = current;
				mxpos = row;
			}

			if (current >= (mx - delta)) continue;

			/* Yeah, we have missed a maximum */
			if (mxpos < row) peaks[col].push_front(peak_t(mxpos, mx));
			else peaks[col].push_back(peak_t(mxpos, mx));

			found = true;
			break;
		}

		if (found) continue;

		/* Nope, there are two minima, delete one! */
		if (peaks[col].begin()->second < peaks[col].end()->second) {
			peaks[col].pop_back();
		}
		else {
			peaks[col].pop_front();
		}
	}
}

size_t ThermalCyclingLifetime::rainflow(const extrema_t &extrema)
{
	size_t extremum_count = extrema.size();

	if (extremum_count > MAX_EXTREMA)
		throw std::runtime_error("There are too many extrema.");

	int i, j;
	size_t count = 0;
	double amplitude, mean;
	extrema_t::const_iterator it = extrema.begin(), itend = extrema.end();

	for (j = -1; it != itend; it++) {
		a[++j] = it->second;

		while ((j >= 2) && (fabs(a[j-1] - a[j-2]) <= fabs(a[j] - a[j-1]))) {
			amplitude = fabs((a[j-1] - a[j-2]) / 2);

			switch (j) {
			case 0: break;
			case 1: break;
			case 2:
				mean = (a[0] + a[1]) / 2;
				a[0] = a[1];
				a[1] = a[2];
				j = 1;
				if (amplitude > 0) {
					/* 0.5 cycles */
					amplitudes[count] = amplitude;
					means[count] = mean;
					cycles[count] = 0.5;
					count++;
				}
				break;

			default:
				mean = (a[j-1] + a[j-2]) / 2;
				a[j-2] = a[j];
				j = j - 2;
				if (amplitude > 0) {
					/* 1 cycle */
					amplitudes[count] = amplitude;
					means[count] = mean;
					cycles[count] = 1;
					count++;
				}
				break;
			}
		}
	}

	for (i = 0; i < j; i++) {
		amplitude = fabs(a[i] - a[i+1]) / 2;
		mean = (a[i] + a[i+1]) / 2;
		if (amplitude > 0) {
			/* 0.5 cycles */
			amplitudes[count] = amplitude;
			means[count] = mean;
			cycles[count] = 0.5;
			count++;
		}
	}

	if ((extremum_count % 2) == 0) {
		/* We are missing something... */

		it = extrema.begin();
		itend--;

		amplitudes[count] = fabs(it->second - itend->second) / 2.0;
		means[count] = (it->second + itend->second) / 2.0;
		cycles[count] = 0.5;

		count++;
	}

	return count;
}

double CombinedThermalCyclingLifetime::predict(
	const matrix_t &temperature, double sampling_interval)
{
	const double Q = q; /* Some stupid joke */

	size_t processor_count = temperature.cols();
	size_t step_count = temperature.rows();

	/* Get extrema */
	std::vector<extrema_t> peaks;
	detect_peaks(temperature, peaks);

	double damage, factor = 0;

	/* For each temperature curve */
	for (size_t i = 0; i < processor_count; i++) {
		size_t cycle_count = rainflow(peaks[i]);

		damage = 0;

		for (size_t j = 0; j < cycle_count; j++) {
			double dT = 2 * amplitudes[j];

			/* Skip cycles that do not cause any damage */
			if (dT <= dT0) continue;

			/* Maximal temperatures during each cycle */
			double Tmax = means[j] + amplitudes[j];

			/* Number of cycles to failure for each stress level [3] */
			double N = Atc * pow(dT - dT0, -Q) * exp(Eatc / (k * Tmax));

			damage += cycles[j] / N;
		}

		factor += pow(damage, beta);
	}

	damage = pow(factor, 1.0 / beta);

	return (sampling_interval * step_count) / damage;
}
