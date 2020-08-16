#include "lagrangian.hpp"

#include <algorithm>
#include <limits>
#include "../core/mergers.hpp"
#include "../core/solver.hpp"
#include "lg_master_subgradient.hpp"
#include "lagrangian_cb.hpp"

#ifdef SOLVER_CPLEX
#include "lg_master_cplex.hpp"
#endif


void LagrangianRelaxation::solve(LagrangianRelaxationParams params)
{
	if (master == NULL || subproblem == NULL) {
		cout << "Error: Lagrangian relaxation called without a model" << endl;
		exit(1);
	}

	int nrows_lag = master->get_nrows_lag();
	vector<double> lambdas(nrows_lag, 0); // Lagrange multipliers, initialized to zero

	double primal_value;
	double dual_value;
	double best_dual_value = numeric_limits<double>::infinity();

	int max_niters = (params.max_niters >= 0) ? params.max_niters : numeric_limits<int>::max();

	Stats stats;
	stats.register_name("subprob");
	stats.register_name("master");

	int it;
	for (it = 0; it < max_niters; ++it) {

		stats.start_timer(0);

		// Obtain optimal solution and value for subproblem
		vector<int> sp_optsol;
		double sp_optval = subproblem->solve(lambdas, sp_optsol);

		stats.end_timer(0);
		stats.start_timer(1);

		// Update master with subproblem solution
		// E.g. If cutting plane method, add cut
		master->update(sp_optsol, sp_optval, lambdas);

		// Update Lagrangian dual and Lagrange multipliers by solving master LP
		primal_value = master->solve(lambdas);

		stats.end_timer(1);

		dual_value = sp_optval;
		if (dual_value < best_dual_value) {
			best_dual_value = dual_value;
		}
		double gap = abs(primal_value - best_dual_value) / (1 + abs(best_dual_value));

		// Print iteration information
		cout << "--- Iteration " << it + 1;
		cout << " -  Primal: " << primal_value;
		cout << " / Dual: " << dual_value;
		cout << " / Best Dual: " << best_dual_value;
		cout << " / Gap: " << gap ;
		cout << " / [Time: M: " << stats.get_time(1) << " / SP: " << stats.get_time(0) << "]";
		cout << endl;

		// Stopping condition
		if (gap <= params.convergence_tol) {
			it++; // for consistency in number of iterations between different stopping conditions
			break;
		}
		if (params.time_limit >= 0 && stats.get_time(0) + stats.get_time(1) >= params.time_limit) {
			cout << "Time limit reached" << endl;
			it++;
			break;
		}
	}

	cout << endl;
	cout << "Number of iterations: " << it << endl;
	cout << "Dual bound: " << best_dual_value << endl;
	cout << "Primal (wrt Lagrangian) bound: " << primal_value << endl;;
	cout << "Total master time: " << stats.get_time(1) << endl;
	cout << "Total subproblem time: " << stats.get_time(0) << endl;

}
