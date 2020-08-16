#include "lagrangian_cb.hpp"

#ifndef USE_CONICBUNDLE

double LagrangianRelaxationCB::solve(LagrangianRelaxationParams params)
{
	cout << "Error: ConicBundle not compiled" << endl;
	exit(1);
}

#else

double LagrangianRelaxationCB::solve(LagrangianRelaxationParams params)
{
	int nrows_lag = relaxed_constrs.size();

	CBSolver solver;
	LagrangianSubproblemCB lsp(relaxed_constrs, subproblem);

	DVector lb(nrows_lag);
	DVector ub(nrows_lag);
	for (int i = 0; i < nrows_lag; ++i) {
		if (relaxed_constrs[i].sense == LINSENSE_LE) {
			lb[i] = 0;
		} else {
			lb[i] = CB_minus_infinity;
		}
		if (relaxed_constrs[i].sense == LINSENSE_GE) {
			ub[i] = 0;
		} else {
			ub[i] = CB_plus_infinity;
		}
	}
	solver.init_problem(nrows_lag, &lb, &ub);
	solver.add_function(lsp);
	// solver.set_out(&cout, 1);
	solver.set_out(&cout, 0);
	solver.set_term_relprec(1e-7);

	if (params.max_noracleiters >= 0) {
		solver.set_eval_limit(params.max_noracleiters);
	}

	int max_niters = (params.max_niters >= 0) ? params.max_niters : numeric_limits<int>::max();
	Stats stats;
	stats.register_name("cb");

	params.time_limit = time_limit;
	stats.start_timer(0);
	int it;
	for (it = 0; it < max_niters; it++) {
		solver.do_descent_step();

		// cout << "Iteration " << it << ": bound " << solver.get_objval() << endl;

		if (DBL_LE(solver.get_objval(), params.obj_limit)) {
			if (options->bounds_verbose) {
				cout << "ConicBundle finished: Objective limit (" << params.obj_limit << ")" << endl;
			}
			break;
		}

		if (solver.termination_code() != 0) {
			if (options->bounds_verbose) {
				cout << "ConicBundle finished: ";
			}
			solver.print_termination_code(cout);
			break;
		}

		if (params.time_limit >= 0 && stats.get_current_time(0) >= params.time_limit) {
			if (options->bounds_verbose) {
				cout << "ConicBundle finished: Time limit (" << params.time_limit << "s)" << endl;
			}
			break;
		}
	}
	stats.end_timer(0);

	// Output
	if (options->bounds_verbose) {
		cout << endl;
		cout << "Number of outer iterations: " << it << endl;
		cout << "Number of inner iterations: " << lsp.get_number_evaluations() << endl;
		// cout << "Dual bound: " << solver.get_objval() << endl;
		// cout << "Primal (wrt Lagrangian) bound: " << primal_value << endl;;
		cout << "Total master time: " << stats.get_time(0) - lsp.get_time() << endl;
		cout << "Total subproblem time: " << lsp.get_time() << endl;
	}

	return solver.get_objval();
}

#endif // USE_CONICBUNDLE
