#include "Lifetime.h"
#include "Architecture.h"
#include "Graph.h"

double ThermalCyclingLifetime::predict(
	const matrix_t &temperature, double sampling_interval)
{
	const double Q = q; /* Some stupid joke */

	size_t processor_count = temperature.cols();
	size_t step_count = temperature.rows();

	double maximal_damage = 0;

	const double *data = temperature;

	/* For each temperature curve */
	for (size_t i = 0; i < processor_count; i++) {
		size_t peak_count = update_peaks(data, step_count, processor_count, i);
		size_t cycle_count = update_cycles(peak_count);

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

#define LF_UNDEFINED 0
#define LF_MIN 1
#define LF_MAX 2

size_t ThermalCyclingLifetime::update_peaks(const double *data,
	size_t rows, size_t cols, size_t col)
{
	size_t count, mxpos, mnpos, row, first_pos = 0, last_pos = 0;
	double current, mx, mn;

	char look_for, first_is;

	mx = mn = data[/* 0 * cols + */ col];
	mxpos = mnpos = 0;

	look_for = first_is = LF_UNDEFINED;

	/* Leave one cell for a possible "push front" */
	peak_index = peaks + 1;
	count = 0;

	/* For each row */
	for (row = 1; row < rows; row++) {
		current = data[row * cols + col];

		if (current >= mx) {
			mx = current;
			mxpos = row;
		}
		if (current <= mn) {
			mn = current;
			mnpos = row;
		}

		if (look_for == LF_MAX) {
			if (current < (mx - delta)) {
				peaks[++count] = mx;
				last_pos = mxpos;

				mn = current;
				mnpos = row;

				look_for = LF_MIN;
			}
		}
		else if (look_for == LF_MIN) {
			if (current > (mn + delta)) {
				peaks[++count] = mn;
				last_pos = mnpos;

				mx = current;
				mxpos = row;

				look_for = LF_MAX;
			}
		}
		else { /* ... undefined so far */
			if (current < (mx - delta)) {
				peaks[++count] = mx;
				last_pos = mxpos;

				mn = current;
				mnpos = row;

				look_for = LF_MIN;

				first_is = LF_MAX;
				first_pos = row;
			}
			else if (current > (mn + delta)) {
				peaks[++count] = mn;
				last_pos = mnpos;

				mx = current;
				mxpos = row;

				look_for = LF_MAX;

				first_is = LF_MIN;
				first_pos = row;
			}
		}
	}

	if (look_for == LF_MAX) {
		/* Ensure that we start from the very beginning */
		if (first_pos > 0) {
			if (first_is == LF_MIN) {
				/* Push front! */
				peak_index--;
				count++;
				peaks[0] = mx;
			}
			else {
				/* Replace! */
				peaks[1] = mx = std::max(mx, peaks[1]);
			}
		}

		/* Ensure that we end in the end */
		if (last_pos < (rows - 1)) {
			peak_index[count++] = mx;
		}
	}
	else { /* ... looking for a minimum */
		/* Ensure that we start from the very beginning */
		if (first_pos > 0) {
			if (first_is == LF_MAX) {
				/* Push front! */
				peak_index--;
				count++;
				peaks[0] = mn;
			}
			else {
				/* Replace! */
				peaks[1] = mn = std::min(mn, peaks[1]);
			}
		}

		/* Ensure that we end in the end */
		if (last_pos < (rows - 1)) {
			peak_index[count++] = mn;
		}
	}

	return count;
}

size_t ThermalCyclingLifetime::update_cycles(size_t extremum_count)
{
	int i, j;
	size_t count = 0;
	double amplitude, mean, *a = temp;

	for (i = 0, j = -1; i < extremum_count; i++, peak_index++) {
		a[++j] = *peak_index;

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

	return count;
}

double CombinedThermalCyclingLifetime::predict(
	const matrix_t &temperature, double sampling_interval)
{
	const double Q = q; /* Some stupid joke */

	size_t processor_count = temperature.cols();
	size_t step_count = temperature.rows();

	double damage, factor = 0;

	const double *data = temperature;

	/* For each temperature curve */
	for (size_t i = 0; i < processor_count; i++) {
		size_t peak_count = update_peaks(data, step_count, processor_count, i);
		size_t cycle_count = update_cycles(peak_count);

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
