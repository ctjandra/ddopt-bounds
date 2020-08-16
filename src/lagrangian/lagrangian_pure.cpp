#include "lagrangian_pure.hpp"
#include "lg_subprob_std.hpp"
#include "lg_constraint_scip.hpp"

/** Given a SCIP model mid-solve, Lagrangianize all constraints and solve via Lagrangian relaxation */
SCIP_RETCODE solve_indepset_lagrangian_pure(SCIP* scip, Options* options)
{
	SCIP_ROW** rows;
	SCIP_COL** cols;
	int nrows;
	int ncols;

	SCIP_CALL(SCIPgetLPRowsData(scip, &rows, &nrows));
	SCIP_CALL(SCIPgetLPColsData(scip, &cols, &ncols));

	// Add all constraints as relaxed Lagrangian constraints
	vector<LagrangianConstraint> relaxed_constrs = lagrangian_extract_all_constraints_scip(scip, rows, nrows);

	// Create subproblem for Lagrangian relaxation
	vector<double> obj(ncols);
	for (int i = 0; i < ncols; ++i) {
		int idx = SCIPvarGetProbindex(SCIPcolGetVar(cols[i]));
		assert(idx < ncols);
		obj[idx] = SCIPcolGetObj(cols[i]);
		obj[idx] = -obj[idx]; // switch signs because SCIP minimizes and we maximize
	}

	// // Debugging info
	// cout << "Relaxed constraints:" << endl;
	// for (int i = 0; i < (int) relaxed_constrs.size(); ++i) {
	// 	cout << "Constr " << i << ": ";
	// 	cout << relaxed_constrs[i] << endl;
	// }
	// cout << "Objective: ";
	// for (int i = 0; i < ncols; ++i) {
	// 	cout << obj[i] << " ";
	// }
	// cout << endl;

	LagrangianSubproblemOracle* oracle = new LagrangianSubproblemOracleZeroOne();
	LagrangianSubproblem* subproblem = new LagrangianSubproblemStandard(ncols, obj, relaxed_constrs, oracle);

	Stats stats;
	stats.register_name("lr_time");
	stats.start_timer(0);

	// Solve Lagrangian relaxation
	LagrangianRelaxationCB lagrangian(relaxed_constrs, subproblem, options->lag_cb_time_limit, options);
	LagrangianRelaxationParams params;
	double objval = lagrangian.solve(params);

	cout << "Retransformed objective: " << -SCIPretransformObj(scip, -objval) << endl;

	stats.end_timer(0);
	cout << "Total Lagrangian time: " << stats.get_time(0) << endl;

	delete subproblem;
	delete oracle;

	return SCIP_OKAY;
}


