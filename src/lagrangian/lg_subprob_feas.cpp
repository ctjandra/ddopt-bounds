#include "lg_subprob_feas.hpp"
#include "lg_dd_selector_scip.hpp"

double LagrangianSubproblemOracleFeasibilityCheck::solve(const vector<double>& obj, vector<int>& optsol)
{
	double optval = oracle->solve(obj, optsol);

	// If no solution is found, do nothing
	if (optsol.size() == 0) {
		return optval;
	}

	// Construct actual solution with fixed variables
	vector<int> true_optsol(fixed_vars);
	assert(true_obj.size() == optsol.size());
	assert(fixed_vars.size() == optsol.size());
	int nvars = optsol.size();
	for (int i = 0; i < nvars; ++i) {
		if (fixed_vars[i] == DD_UNFIXED_VAR) {
			true_optsol[i] = optsol[i];
		} else {
			assert(optsol[i] == 0); // This is a check specific to using subspace with zero objective for fixed variables
		}
	}

	// Check (true) objective value of oracle solution
	double true_optval = get_optimal_value(true_obj, true_optsol);

	// If true objective value is better than primal bound, check feasibility
	if (DBL_LT(-true_optval, primal_bound)) {
		// cout << "Potential primal improvement: " << -true_optval << " better than " << primal_bound << endl;
		bool feasible = feas_checker->check_feasibility_and_apply(true_optsol, -true_optval);

		// If feasible, mark as new primal bound
		if (feasible) {
			if (options->bounds_verbose) {
				cout << "Feasible primal improvement found: from " << primal_bound << " to " << -true_optval << endl;
			}
			output_stats->num_primal_improved++;
			primal_bound = -true_optval;
		}
	}

	// // Debugging info
	// cout << "Solving Oracle: ";
	// cout << "size " << optsol.size() << endl;
	// // cout << "   True Obj: ";
	// // for (double k : true_obj) {
	// // 	cout << k << " ";
	// // }
	// // cout << endl;
	// cout << "   True Sol: ";
	// for (int k : true_optsol) {
	// 	cout << k << " ";
	// }
	// cout << endl;
	// cout << "   True Val: " << true_optval << endl;
	// cout << "Primal Bound: " << primal_bound << endl;

	return optval;
}
