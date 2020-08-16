/* Master problem for Lagrangian relaxation: Simple subgradient method */

#include "lg_master_subgradient.hpp"


void LagrangianMasterProblemSubgradient::update(const vector<int>& sp_optsol, double sp_optval, const vector<double>& lambdas)
{
	// Prepare values for solve

	cout << "Updating master with optimal value " << sp_optval << " and solution: ";
	for (int v : sp_optsol) {
		cout << v << " ";
	}
	cout << endl;

	int nrows_lag = get_nrows_lag();
	for (int i = 0; i < nrows_lag; ++i) {
		current_subgradients[i] = relaxed_constrs[i].get_lagrangian_subgradient(sp_optsol);
	}

	stepsize = compute_stepsize(sp_optval);

	cout << "Current step size: " << stepsize << endl;
}

double LagrangianMasterProblemSubgradient::compute_stepsize(double current_lag_dual)
{
	// Step size calculation from Fisher (1985)
	if (DBL_LT(current_lag_dual, best_lag_dual)) {
		best_lag_dual = current_lag_dual;
		nstalls = 0;
	} else {
		if (nstalls < 5) {
			nstalls++;
		} else {
			// Divide scaling factor by two if no improvement for a number of iterations
			stepscale *= 0.5;
			nstalls = 0;
		}
	}

	double denom = 0.0;
	int nrows_lag = get_nrows_lag();
	for (int i = 0; i < nrows_lag; ++i) {
		denom += current_subgradients[i] * current_subgradients[i];
	}

	double stepsz;
	stepsz = 1.0 / ((double) ncalls + 1);

	return stepsz;
}

double LagrangianMasterProblemSubgradient::solve(vector<double>& lambdas)
{
	ncalls++;

	int size = lambdas.size();
	for (int i = 0; i < size; ++i) {
		// We assume original problem is a maximization problem
		lambdas[i] -= stepsize * current_subgradients[i];
	}

	cout << "Updated lambdas to ";
	for (int i = 0; i < size; ++i) {
		cout << lambdas[i] << " ";
	}
	cout << endl;
	// cout << "Master objective: " << optval << endl;

	return 0;
}
