#include "Lifetime.h"
#include "Architecture.h"
#include "Hotspot.h"
#include "DynamicPower.h"
#include "Graph.h"

double Lifetime::predict(const matrix_t &temperature, double sampling_interval)
{
	double time = sampling_interval * temperature.rows();
	double damage = calc_damage(temperature);

	return time / damage;
}

double Lifetime::calc_damage(const matrix_t &temperature)
{
	size_t i, j;
	std::vector<extrema_t> peaks;
	vector_t amplitudes, means;

	double damage, factor, tmax, n;
	const double Q = q; /* Some stupid joke */

	/* Get extrema */
	detect_peaks(temperature, peaks);

	factor = 0;

	/* For each temperature curve */
	size_t cols = temperature.cols();
	for (i = 0; i < cols; i++) {
		rainflow(peaks[i], amplitudes, means);

		damage = 0;

		size_t cycle_count = amplitudes.size();
		for (j = 0; j < cycle_count; j++) {
			/* Maximal temperatures during each cycle */
			tmax = means[j] + amplitudes[j] / 2.0;

			/* Number of cycles to failure for each stress level [3] */
			n = Atc * pow(amplitudes[j] - dT0, -Q) * exp(Eatc / (k * tmax));

			/* Count all detected cycles (even 0.5) as completed,
			 * since we have cycling temperature fluctuations
			 * (1 instead of cycles here).
			 */
			damage += 1.0 / n;
		}

		factor += pow(damage, beta);
	}

	damage = pow(factor, 1.0 / beta);

	return damage;
}

void Lifetime::detect_peaks(const matrix_t &data, std::vector<extrema_t> &peaks)
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

void Lifetime::rainflow(const extrema_t &extrema,
	vector_t &amplitudes, vector_t &means)
{
	int i, j;
	double amplitude, mean;

	size_t extremum_count = extrema.size();
	std::vector<double> a(extremum_count);

	extrema_t::const_iterator it, itend;

	amplitudes.clear();
	means.clear();

	itend = extrema.end();
	for (it = extrema.begin(), j = -1; it != itend; it++) {
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
					means.push_back(mean);
					amplitudes.push_back(amplitude);
				}
				break;

			default:
				mean = (a[j-1] + a[j-2]) / 2;
				a[j-2] = a[j];
				j = j - 2;
				if (amplitude > 0) {
					/* 1 cycle */
					means.push_back(mean);
					amplitudes.push_back(amplitude);
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
			means.push_back(mean);
			amplitudes.push_back(amplitude);
		}
	}
}
