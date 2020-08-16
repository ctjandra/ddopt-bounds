#include "lg_subprob_std.hpp"

double LagrangianSubproblemStandard::solve(const vector<double>& lambdas, vector<int>& optsol)
{
	int nconstrs = lambdas.size();
	assert(nconstrs == (int) relaxed_constrs.size());

	// Objective coefficients of Lagrangian relaxation: c - A^T lambda
	vector<double> lag_obj(nvars, 0);

	// c
	for (int i = 0; i < nvars; ++i) {
		lag_obj[i] = obj[i];
	}

	// - A^T lambda
	for (int j = 0; j < nconstrs; ++j) {
		int nnonz = relaxed_constrs[j].nnonz;
		for (int k = 0; k < nnonz; ++k) {
			int var = relaxed_constrs[j].ind[k];
			double coeff = relaxed_constrs[j].coeffs[k];
			lag_obj[var] -= coeff * lambdas[j];
		}
	}

	// Calculate optimal solution using the oracle
	double oracle_optval = oracle->solve(lag_obj, optsol);

	// Add lambda^T b to optimal value
	double optimal_value = oracle_optval;
	for (int j = 0; j < nconstrs; ++j) {
		optimal_value += lambdas[j] * relaxed_constrs[j].rhs;
	}

	// // Debugging info
	// cout << "Original obj coeffs: ";
	// for (int i = 0; i < nvars; ++i) {
	// 	cout << obj[i] << " ";
	// }
	// cout << endl;
	// cout << "Obj coeffs: ";
	// for (int i = 0; i < nvars; ++i) {
	// 	cout << lag_obj[i] << " ";
	// }
	// cout << endl;
	// cout << "Lambdas: ";
	// for (int j = 0; j < nconstrs; ++j) {
	// 	cout << lambdas[j] << " ";
	// }
	// cout << endl;
	// cout << "Subproblem solved with optimal value " << optimal_value;
	// cout << " / Oracle optimal value: " << oracle_optval << endl;
	// for (int v : optsol) {
	// 	cout << v << " ";
	// }
	// cout << endl;

	return optimal_value;
}
