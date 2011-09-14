#include <mex.h>
#include <mex_utils.h>
#include <Architecture.h>
#include <Hotspot.h>

#include <vector>
#include <string>

using namespace std;

ArchitectureBuilder construct_architecture(size_t processor_count,
	const double *_voltage, const double *_ngate)
{
	vector<double> frequency(processor_count, 0);
	vector<double> voltage(processor_count);
	vector<unsigned long int> ngate(processor_count);
	vector<vector<unsigned long int> > nc(processor_count);
	vector<vector<double> > ceff(processor_count);

	for (size_t i = 0; i < processor_count; i++) {
		voltage[i] = _voltage[i];
		ngate[i] = _ngate[i];
	}

	return ArchitectureBuilder(frequency, voltage, ngate, nc, ceff);
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	char *floorplan, *config;
	verify_and_fetch_properties(nrhs, prhs, &floorplan, &config);

	if (nrhs < 2 || mxIsEmpty(prhs[1])) mexErrMsgTxt(
		"The second input should be the dynamic power profile.");

	int step_count = mxGetM(prhs[1]);
	int processor_count = mxGetN(prhs[1]);
	double *_dynamic_power = mxGetPr(prhs[1]);

	if (nrhs < 3 || mxIsEmpty(prhs[2])) mexErrMsgTxt(
		"The third input should be a vector of the voltage supply.");
	if (mxGetN(prhs[2]) * mxGetM(prhs[2]) != processor_count) mexErrMsgTxt(
		"Dimensions of the power profile and the voltage supply should agree.");
	double *voltage = mxGetPr(prhs[2]);

	if (nrhs < 4 || mxIsEmpty(prhs[3])) mexErrMsgTxt(
		"The fourth input should be a vector of the number of gates.");
	if (mxGetN(prhs[3]) * mxGetM(prhs[3]) != processor_count) mexErrMsgTxt(
		"Dimensions of the power profile and the number of gates should agree.");
	double *ngate = mxGetPr(prhs[3]);

	double tol = 0.01;
	if (nrhs > 4) {
		if (!mxIsNumeric(prhs[4])) mexErrMsgTxt(
			"The fifth input should be numeric (tol).");
		tol = mxGetScalar(prhs[4]);
	}

	int maxit = 10;
	if (nrhs > 5) {
		if (!mxIsNumeric(prhs[5])) mexErrMsgTxt(
			"The sixth input should be numeric (maxit).");
		maxit = (int)mxGetScalar(prhs[5]);
	}

	int tsize = 0;
	str_pair *table = NULL;
	if (nrhs > 6) {
		if (!mxIsStruct(prhs[6])) mexErrMsgTxt(
			"The seventh input should be a structure (config).");
		tsize = parse_structure_config(prhs[6], &table);
		if (!table || !tsize) mexErrMsgTxt(
			"The format of the configuration structure is wrong.");
	}

    mxArray *out_temperature = mxCreateDoubleMatrix(
		step_count, processor_count, mxREAL);
	double *_temperature = mxGetPr(out_temperature);

	ArchitectureBuilder architecture = construct_architecture(processor_count,
		voltage, ngate);
	Hotspot hotspot(string(floorplan), string(config), table, tsize);

	if (table) mxFree(table);
	mxFree(floorplan);
	mxFree(config);

	matrix_t dynamic_power(step_count, processor_count);
	matrix_t temperature;
	matrix_t total_power;

	mex_matrix_to_c(dynamic_power.pointer(), _dynamic_power,
		step_count, processor_count);

	size_t it = hotspot.solve(&architecture, dynamic_power,
		temperature, total_power, tol, maxit);

	if (it < 0) {
		mxDestroyArray(out_temperature);
		mexErrMsgIdAndTxt("Hotspot:solve_condensed_equation_with_leakage",
			"Cannot solve using the condensed equation method (%d).", it);
	}

	c_matrix_to_mex(_dynamic_power, total_power.pointer(),
		step_count, processor_count);
	c_matrix_to_mex(_temperature, temperature.pointer(),
		step_count, processor_count);

	mxArray *out_it = mxCreateDoubleMatrix(1, 1, mxREAL);
	*mxGetPr(out_it) = it;

    plhs[0] = out_temperature;
    plhs[1] = out_it;
}
