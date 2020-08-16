/**
 * Decision diagram bounds for SCIP
 * @file   relax_dd.cpp
 * @brief  dd relaxator
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>

#include "relax_dd.h"

#include "../core/solver.hpp"
#include "../util/stats.hpp"

#include "../problem/bp/bp_state.hpp"
#include "../problem/bp/bp_reader_scip.hpp"
#include "../problem/bp/bprow.hpp"
#include "../problem/bp/prop_linearcons.hpp"
#include "../problem/bp/filtering.hpp"

#include "../lagrangian/lagrangian_pure.hpp"
#include "../lagrangian/lg_subprob.hpp"
#include "../lagrangian/lg_subprob_bdd.hpp"
#include "../lagrangian/lg_subprob_feas.hpp"
#include "../lagrangian/lg_subprob_feas_scip.hpp"
#include "../lagrangian/lg_subprob_std.hpp"
#include "../lagrangian/lg_subprob_subrelaxed.hpp"
#include "../lagrangian/lg_subprob_nrp.hpp"
#include "../lagrangian/lg_constraint_scip.hpp"
#include "../lagrangian/lg_dd_selector_ct_scip.hpp"
#include "../lagrangian/lg_dd_selector_bp_scip.hpp"

#include "../problem/cliquetable/cliquetable_cons_id.hpp"
#include "../problem/cliquetable/cliquetable_instance.hpp"
#include "../problem/cliquetable/cliquetable_problem.hpp"
#include "../problem/cliquetable/cliquetable_orderings.hpp"
#include "../problem/cliquetable/cliquetable_completion.hpp"
#include "../problem/cliquetable/ct_prop_linearcons.hpp"

#include "scip/struct_scip.h"
#include "scip/struct_lp.h"
#include "scip/var.h"
#include "scip/set.h"
#include "scip/clock.h"

#define RELAX_NAME             "dd"
#define RELAX_DESC             "relaxed decision diagrams for binary programs"
// #define RELAX_PRIORITY         100000
#define RELAX_PRIORITY         -1   // after LPs and propagations
#define RELAX_FREQ             1

#define UNSET_VAL              -1  /* assumes variables are nonnegative */



/*
 * Data structures
 */

/** relaxator data */
struct SCIP_RelaxData {
	Options*              options;            /**< options */
	OutputStats*          output_stats;       /**< output statistics */

	SCIP_CLOCK*           relaxatorclock;     /**< clock for timing relaxator */
	int                   depthfreq;          /**< frequency per depth the relaxator is called */
	SCIP_Bool             primalatexact;      /**< only generate primal feasible solution when BDD is exact */
	SCIP_Bool             prunebdd;           /**< use primal bound to prune BDD */

	SCIP_CLOCK*           profilingclock;     /**< clock used for profiling */

	int                   nruns;

	SCIPRowVector*        lagrangian_rows;    /**< rows to put in Lagrangian relaxation; constructed at start */
	bool                  disabled;           /**< if true, this relaxator does not run anymore */
	bool                  firstrun;           /**< if true, the current run is the first run */
	LagrangianDDConstraintSelector* lag_selector;  /**< constraint selector for Lagrangian relaxation and DD construction */
};


/*
 * Main methods
 */

bool is_constraint_redundant_to_dd(BDD* bdd, LagrangianConstraint constr, const vector<int>& var_to_subvar)
{
	int nvars = bdd->nvars();
	vector<int> optimal_sol;
	vector<double> weights(nvars, 0);
	double rhs = 0;

	if (constr.sense == LINSENSE_LE) {
		rhs = constr.rhs;
		for (int i = 0; i < constr.nnonz; ++i) {
			assert(var_to_subvar[constr.ind[i]] <= nvars);
			weights[var_to_subvar[constr.ind[i]]] = constr.coeffs[i];
		}
	} else if (constr.sense == LINSENSE_GE) { // switch all signs
		rhs = -constr.rhs;
		for (int i = 0; i < constr.nnonz; ++i) {
			assert(var_to_subvar[constr.ind[i]] <= nvars);
			weights[var_to_subvar[constr.ind[i]]] = -constr.coeffs[i];
		}
	} else {
		assert(false); // not yet implemented
	}

	double bound = bdd->get_optimal_sol(weights, optimal_sol, true);
	cout << "Testing redundancy of " << constr << " -- bound " << bound << endl;

	if (DBL_LE(bound, rhs)) {
		return true;
	} else {
		return false;
	}
}


vector<double> get_scip_objective_for_dd(SCIP* scip, SCIP_COL** cols, int ncols)
{
	vector<double> obj(ncols);
	for (int i = 0; i < ncols; ++i) {
		int idx = SCIPvarGetProbindex(SCIPcolGetVar(cols[i]));
		assert(idx < ncols);
		obj[idx] = SCIPcolGetObj(cols[i]);
		obj[idx] = -obj[idx]; // switch signs because SCIP minimizes and we maximize
		// Note: To obtain original objective value, we will need to apply SCIPretransformObj(scip, obj) to the final value
	}
	return obj;
}


double get_scip_objective_constant_for_fixed_vars(SCIP* scip, SCIP_COL** cols, int ncols, const vector<int>& fixed_vars)
{
	double objconstant = 0.0;
	for (int i = 0; i < ncols; ++i) {
		SCIP_VAR* scip_var = SCIPcolGetVar(cols[i]);
		int idx = SCIPvarGetProbindex(scip_var);
		assert(idx < ncols);
		if (fixed_vars[idx] >= 0) { // if fixed
			double curobj = SCIPcolGetObj(cols[i]);
			curobj = -curobj; // switch signs because SCIP minimizes and we maximize
			objconstant += curobj * fixed_vars[idx];
			// Note: To obtain original objective value, we will need to apply SCIPretransformObj(scip, obj) to the final value
		}
	}
	return objconstant;
}


vector<bool> get_fixed_variables(SCIP* scip, SCIP_COL** cols, int ncols)
{
	vector<bool> fixed_vars;

	for (int j = 0; j < ncols; ++j) {
		SCIP_VAR* scip_var = SCIPcolGetVar(cols[j]);
		int idx = SCIPvarGetProbindex(scip_var);
		assert(idx >= 0 && idx < ncols);
		if (SCIPisEQ(scip, SCIPvarGetLbLocal(scip_var), SCIPvarGetUbLocal(scip_var))) {
			fixed_vars[idx] = true;
		} else {
			fixed_vars[idx] = false;
		}
	}

	return fixed_vars;
}


SCIP_RETCODE construct_dd_from_bp_lag(SCIP* scip, Options* options, OutputStats* output_stats, double* dualbound,
								      SCIPRowVector* lagrangian_rows, LagrangianDDConstraintSelector* lag_selector)
{
	SCIP_ROW** rows;
	SCIP_COL** cols;
	int nrows;
	int ncols;
	Stats stats;

	stats.register_name("dd_time");
	stats.start_timer(0);

	stats.register_name("full_time");
	stats.start_timer(1);

	SCIP_CALL(SCIPgetLPRowsData(scip, &rows, &nrows));
	SCIP_CALL(SCIPgetLPColsData(scip, &cols, &ncols));

	vector<int> var_to_subvar;
	vector<int> subvar_to_var;

	// Retrieve fixed variables and the values they are fixed to from SCIP
	// (This may be expanded in preprocess_fixed_vars and we should not retrieve this information from SCIP from now on)
	vector<int> fixed_vars = get_fixed_variables_binary_scip(scip, cols, ncols);

	// Expand the set of fixed variables if possible; these must be valid for the entire subproblem and not only the DD constraints
	lag_selector->preprocess_fixed_vars(scip, fixed_vars);

	// Create mappings between original space and subspace of unfixed variables
	lag_selector->create_mappings(scip, fixed_vars, var_to_subvar, subvar_to_var);

	// Subspace only includes unfixed variables, but it may contain even less since it ignores variables not in
	// any relevant constraint for the DD. Note that these unfixed variables are only ignored for the DD step, and not
	// in the Lagrangian relaxation.
	assert((int) subvar_to_var.size() <= count(fixed_vars.begin(), fixed_vars.end(), DD_UNFIXED_VAR));

	// Get objective
	vector<double> sub_obj = get_scip_objective_for_dd(scip, cols, ncols);
	vector<double> full_obj = sub_obj;
	double objconstant = get_scip_objective_constant_for_fixed_vars(scip, cols, ncols, fixed_vars);

	// Set fixed vars objective to zero; these are handled separately (objconstant)
	// Note: This makes the values of optsol (further below) for fixed variables become junk, but ok because we do not use it
	for (int i = 0; i < ncols; ++i) {
		if (fixed_vars[i] != DD_UNFIXED_VAR) {
			sub_obj[i] = 0;
		}
	}

	SCIP_Real t_primal_bound = SCIPgetUpperbound(scip);

	SCIP_Real primal_bound = SCIPgetPrimalbound(scip);

 	// primal bound taking into account transformations and only variables in subspace
 	double subspace_primal_bound = -t_primal_bound - objconstant;

	// Construct decision diagram
	DDSolver* solver = lag_selector->create_solver(scip, var_to_subvar, subvar_to_var, fixed_vars, sub_obj, options);

	// Primal pruning
	if (options->lag_primal_pruning) {
		if (!options->lag_pure_bp) {
			solver->problem->completion = new CliqueTableDomainCompletionBound();
			// cout << "Primal bound set for DD: " << subspace_primal_bound << endl;
			solver->set_primal_bound(subspace_primal_bound);
		} else {
			cout << "Warning: Primal pruning unsupported for non-clique table" << endl;
		}
	}

	// Dual pruning
	if (options->lag_dual_pruning) {
		// dual bound taking into account transformations and only variables in subspace
		double subspace_dual_bound = -SCIPgetLocalLowerbound(scip) - objconstant;

		if (!options->lag_pure_bp) {
			solver->problem->completion = new CliqueTableDomainCompletionBound();
			// cout << "Dual bound set for DD: " << subspace_dual_bound << endl;
			solver->set_dual_bound(subspace_dual_bound);
		} else {
			cout << "Warning: Dual pruning unsupported for non-clique table" << endl;
		}
	}

	BDD* bdd = solver->construct_decision_diagram(scip);

	stats.end_timer(0);
	double bdd_time = stats.get_time(0);

	output_stats->bdd_time += bdd_time;
	if (bdd == NULL || solver->final_exact) {
		output_stats->num_bdd_exact++;
	}

	if (SCIPisStopped(scip)) {
		return SCIP_OKAY;
	}

	// If BDD infeasible, then we can set the dual bound to -infinity
	if (bdd == NULL) {
		stats.end_timer(1);
		if (options->bounds_verbose) {
			cout << "BDD is infeasible" << endl;
		}
		*dualbound = -SCIPinfinity(scip);
		return SCIP_OKAY;
	}

	// Primal bound by non-relaxed path
	if (options->lag_generate_primal_nrp) {
		LagrangianSubproblemOracle* primal_oracle = new LagrangianSubproblemOracleNRP(bdd);
		primal_oracle = new LagrangianSubproblemOracleSubspaceRelaxed(primal_oracle, subvar_to_var);
		primal_oracle = new LagrangianSubproblemOracleFeasibilityCheck(primal_oracle,
		        new FeasibilityCheckerSCIP(scip), full_obj, fixed_vars, primal_bound, options, output_stats);
		vector<int> primal_sol;
		primal_oracle->solve(sub_obj, primal_sol);
	}

	// Set up Lagrangian relaxation

	// Note that the oracle must be relaxed to capture unfixed isolated variables outside subspace; fixed variables are handled
	// through the zero objective
	LagrangianSubproblemOracle* oracle = new LagrangianSubproblemOracleSubspaceRelaxed(
	    new LagrangianSubproblemOracleBDD(bdd),
	    subvar_to_var);

	if (options->lag_generate_primal) {
		oracle = new LagrangianSubproblemOracleFeasibilityCheck(oracle, new FeasibilityCheckerSCIP(scip), full_obj,
		        fixed_vars, primal_bound, options, output_stats);
	}

	vector<int> optsol;
	double optval = oracle->solve(sub_obj, optsol);
	// Warning: The values of optsol outside the subspace are junk because we set their objective to zero above
	// optval should be equal to bdd->bound converted from the subspace

	if (options->bounds_verbose) {
		cout << endl;
		cout << "BDD bound: " << optval << endl;
		cout << "BDD width: " << solver->final_width << endl;
		cout << "BDD time: " << bdd_time << endl;
		cout << endl;
	}

	// cout << "Full DD:" << endl;
	// bdd->print(false, true);

	// DD bound
	*dualbound = optval + objconstant;
	*dualbound += 1e-6; // relaxation constant for safety purposes (assuming minimization)

	if (options->bounds_verbose) {
		cout << "BDD bound (transformed): " << *dualbound << endl;
		cout << endl;
	}

	// If time or iteration limit is zero, or no Lagrangian constraints:
	// Do not solve Lagrangian relaxation; instead, only output the bound from the BDD
	if (DBL_EQ(options->lag_cb_time_limit, 0) || options->lag_cb_iter_limit == 0 || lagrangian_rows->size() == 0) {
		stats.end_timer(1);
		if (options->bounds_verbose) {
			cout << "Dual bound: " << *dualbound << "   [Objective constant: " << objconstant << "]" << endl;
		}
		delete bdd;
		delete solver->problem->inst;
		delete solver->problem;
		delete solver;
		delete oracle;
		return SCIP_OKAY;
	}

	// If dual bound is already better than primal bound, do not solve Lagrangian relaxation
	if (DBL_LT(t_primal_bound, -(*dualbound))) {
		stats.end_timer(1);
		if (options->bounds_verbose) {
			cout << "DD bound already prunes node; Lagrangian relaxation skipped" << endl;
			cout << "Dual bound: " << *dualbound << "   [Objective constant: " << objconstant << "]" << endl;
		}
		delete bdd;
		delete solver->problem->inst;
		delete solver->problem;
		delete solver;
		delete oracle;
		return SCIP_OKAY;
	}

	// Create set of relaxed Lagrangian constraints, which take the fixed variables into account
	vector<LagrangianConstraint> relaxed_constrs = lagrangian_extract_constraints_scip(scip, lagrangian_rows, true, fixed_vars);

	// // Debugging info
	// cout << "Relaxed constraints:" << endl;
	// for (int i = 0; i < (int) relaxed_constrs.size(); ++i) {
	//    cout << "Constr " << i << ":  " << relaxed_constrs[i] << endl;
	// }

	// Create subproblem for Lagrangian relaxation
	LagrangianSubproblem* subproblem = new LagrangianSubproblemStandard(ncols, sub_obj, relaxed_constrs, oracle);


	// Solve Lagrangian relaxation

	// The Lagrangian relaxation is done in the full space of variables, but the objectives of fixed variables are set to zero.
	// Note that there should not be a significant performance issue (relative to working in the subspace strictly) because
	// the Lagrangian relaxation works in the dual space, and the space of variables is only relevant when calling the subproblem,
	// which works in the restricted subspace.

	stats.register_name("lr_time");
	stats.start_timer(2);

	LagrangianRelaxationCB lagrangian(relaxed_constrs, subproblem, options->lag_cb_time_limit, options);
	LagrangianRelaxationParams params;
	params.obj_limit = subspace_primal_bound;
	params.max_noracleiters = options->lag_cb_iter_limit;
	*dualbound = lagrangian.solve(params);
	*dualbound += objconstant;
	*dualbound += 1e-6; // relaxation constant for safety purposes (assuming minimization)

	if (options->bounds_verbose) {
		cout << "Dual bound: " << -SCIPretransformObj(scip, -(*dualbound)) << "   [Objective constant: " << objconstant << "]" << endl;
	}

	stats.end_timer(1);
	stats.end_timer(2);
	if (options->bounds_verbose) {
		cout << "Total Lagrangian time: " << stats.get_time(2) << endl;
	}

	delete bdd;
	delete solver->problem->inst;
	delete solver->problem;
	delete solver;
	delete subproblem;
	delete oracle;

	return SCIP_OKAY;
}


int count_number_of_free_variables(SCIP* scip, SCIP_COL** cols, int ncols)
{
	// Count number of fixed variables
	int nfixed = 0;
	for (int j = 0; j < ncols; ++j) {
		SCIP_VAR* scip_var = SCIPcolGetVar(cols[j]);
		assert(SCIPvarGetProbindex(scip_var) >= 0 && SCIPvarGetProbindex(scip_var) < ncols);
		if (SCIPisEQ(scip, SCIPvarGetLbLocal(scip_var), SCIPvarGetUbLocal(scip_var))) {
			nfixed++;
		}
	}

	int nfree = ncols - nfixed;

	return nfree;
}


/** Solve sub-MIP; for debugging purposes (e.g. to check if bounds generated are valid) */
static
SCIP_RETCODE solve_submip(SCIP* scip, SCIP_Real* bound)
{
	SCIP* subscip;
	SCIP_VAR** vars;
	SCIP_VAR** subvars;
	SCIP_HASHMAP* varmap;
	SCIP_Bool valid = FALSE;
	int nvars;

	SCIP_CALL(SCIPgetVarsData(scip, &vars, &nvars, NULL, NULL, NULL, NULL));
	SCIP_CALL(SCIPcreate(&subscip));
	SCIP_CALL(SCIPallocBufferArray(scip, &subvars, nvars));
	SCIP_CALL(SCIPhashmapCreate(&varmap, SCIPblkmem(subscip), 5 * nvars));
	SCIP_CALL(SCIPcopy(scip, subscip, varmap, NULL, "_sub", FALSE, FALSE, TRUE, &valid));

	for (int i = 0; i < nvars; i++) {
		subvars[i] = (SCIP_VAR*) SCIPhashmapGetImage(varmap, vars[i]);
	}
	SCIPhashmapFree(&varmap);

	SCIP_CALL(SCIPsetIntParam(subscip, "display/verblevel", 0));
	SCIP_CALL(SCIPsetSubscipsOff(subscip, TRUE));
	SCIP_CALL(SCIPsetSeparating(subscip, SCIP_PARAMSETTING_OFF, TRUE));
	SCIP_CALL(SCIPsetPresolving(subscip, SCIP_PARAMSETTING_FAST, TRUE));

	// SCIPprintOrigProblem(subscip, NULL, NULL, FALSE);

	SCIP_CALL(SCIPsolve(subscip));

	SCIP_Real primal = SCIPgetPrimalbound(subscip);
	SCIP_Real dual = SCIPgetDualbound(subscip);
	// cout << "[Validation] Subproblem number of solutions: " << SCIPgetNSols(subscip) << endl;
	cout << "[Validation] Subproblem bounds: Primal: " << primal << " / Dual: " << dual << endl;
	if (SCIPgetStatus(subscip) == SCIP_STATUS_INFEASIBLE) {
		cout << "[Validation] Subproblem infeasible" << endl;
	}
	cout << "[Validation] Subproblem time: " << SCIPgetSolvingTime(subscip) << endl;

	// SCIPprintStatistics(subscip, NULL);

	SCIPfreeBufferArray(scip, &subvars);
	SCIP_CALL(SCIPfree(&subscip));

	*bound = primal;

	return SCIP_OKAY;
}


/** Initialize necessary structures for relaxator at the start of the solving process */
static
SCIP_RETCODE init_dd_relaxator(SCIP* scip, SCIP_RELAXDATA* relaxdata)
{
	SCIP_ROW** rows;
	int nrows;
	SCIP_CALL(SCIPgetLPRowsData(scip, &rows, &nrows));

	if (!relaxdata->options->lag_pure_bp) {
		// Clique table
		relaxdata->lag_selector = new LagrangianDDConstraintSelectorCliqueTable();
	} else {
		// Pure BP
		relaxdata->lag_selector = new LagrangianDDConstraintSelectorBP();
	}

	// Disable relaxator if there are no decision diagrams to construct
	if (!relaxdata->lag_selector->exists_structure(scip, rows, nrows)) {
		cout << "Warning: Decision diagram relaxator enabled but no structure found; disabling relaxator" << endl;
		relaxdata->disabled = true;
		return SCIP_OKAY;
	}

	// SCIPprintTransProblem(scip, NULL, NULL, FALSE);

	if (relaxdata->options->lag_add_all_rows) {
		relaxdata->lagrangian_rows = new SCIPRowVector();
		for (int i = 0; i < nrows; ++i) {
			relaxdata->lagrangian_rows->push_back(rows[i], !SCIPisInfinity(scip, -SCIProwGetLhs(rows[i])),
			                                      !SCIPisInfinity(scip, SCIProwGetRhs(rows[i])));
		}
	} else {
		relaxdata->lagrangian_rows = relaxdata->lag_selector->extract_lagrangian_rows(scip, rows, nrows, relaxdata->options);
	}
	relaxdata->lag_selector->prepare_dd_construction(scip, rows, nrows, relaxdata->options);

	if (relaxdata->options->bounds_verbose) {
		cout << "Number of Lagrangian rows: " << relaxdata->lagrangian_rows->size() << endl;
	}

	return SCIP_OKAY;
}


/*
 * Callback methods of relaxator
 */


// Default plugin methods
#define relaxCopyDd NULL
#define relaxInitDd NULL
#define relaxExitDd NULL
#define relaxInitsolDd NULL
#define relaxExitsolDd NULL


/** destructor of relaxator to free user data (called when SCIP is exiting) */
static
SCIP_DECL_RELAXFREE(relaxFreeDd)
{
	/*lint --e{715}*/
	SCIP_RELAXDATA* relaxdata;

	/* free branching rule data */
	relaxdata = SCIPrelaxGetData(relax);
	SCIPclockFree(&relaxdata->relaxatorclock);
	SCIPclockFree(&relaxdata->profilingclock);
	delete relaxdata->lagrangian_rows;
	delete relaxdata->lag_selector;
	SCIPfreeMemory(scip, &relaxdata);
	SCIPrelaxSetData(relax, NULL);

	return SCIP_OKAY;
}


/** execution method of relaxator */
static
SCIP_DECL_RELAXEXEC(relaxExecDd)
{
	assert(relax != NULL);
	assert(strcmp(SCIPrelaxGetName(relax), RELAX_NAME) == 0);
	assert(scip != NULL);
	assert(result != NULL);

	Stats stats;
	SCIP_RELAXDATA* relaxdata = SCIPrelaxGetData(relax);
	assert(relaxdata != NULL);

	if (SCIPisStopped(scip)) {
		return SCIP_OKAY;
	}

	// Construct LP if not constructed so we may obtain rows and columns
	if (!SCIPisLPConstructed(scip)) {
		SCIP_Bool cutoff;
		SCIPconstructLP(scip, &cutoff);
		SCIPflushLP(scip);
	}

	if (relaxdata->disabled) {
		*result = SCIP_DIDNOTRUN;
		return SCIP_OKAY;
	}

	// If first run: Construct Lagrangian rows and other necessary structures to be used throughout solving process
	if (relaxdata->firstrun) {
		relaxdata->firstrun = false;
		SCIP_CALL(init_dd_relaxator(scip, relaxdata));

		if (relaxdata->options->lag_initial_dd) {
			SCIP_CALL(SCIPsetRelaxPriority(scip, relax, -1));   // move relaxator to after LP
		}

		if (relaxdata->disabled) {
			*result = SCIP_DIDNOTRUN;
			return SCIP_OKAY;
		}
	}

	// Only call relaxator after LP is solved
	if (SCIPgetLPSolstat(scip) != SCIP_LPSOLSTAT_OPTIMAL) {
		*result = SCIP_SUSPENDED;
		return SCIP_OKAY;
	}

	assert(relaxdata->lag_selector != NULL);

	// Disable BDD output
	relaxdata->options->quiet = true;

	double dualbound;

	// Skip relaxator if more variables than the specified threshold
	SCIP_COL** cols;
	int ncols;
	SCIP_CALL(SCIPgetLPColsData(scip, &cols, &ncols));

	int nfree = count_number_of_free_variables(scip, cols, ncols);

	if (relaxdata->options->bounds_verbose) {
		cout.precision(12);
		cout << "N. free vars: " << nfree << " / " << ncols;
		cout << " -- id " << SCIPnodeGetNumber(SCIPgetCurrentNode(scip));
		if (SCIPnodeGetParent(SCIPgetCurrentNode(scip)) != NULL) {
			cout << " -- parent " << SCIPnodeGetNumber(SCIPnodeGetParent(SCIPgetCurrentNode(scip)));
		} else {
			cout << " -- parent none";
		}
		cout << " -- depth " << SCIPgetDepth(scip);
		cout << " -- lp " << SCIPgetLocalDualbound(scip);
		cout << " -- prim " << SCIPgetPrimalbound(scip);
		cout << endl;
	}

	relaxdata->output_stats->num_attempts++;

	int nvars_threshold = (int)(relaxdata->options->lag_nvars_frac_to_apply * ncols + 1e-6);
	if (relaxdata->options->lag_nvars_to_apply >= 0) {
		// Assume only one between fraction or number of variables is set
		nvars_threshold = relaxdata->options->lag_nvars_to_apply;
	}
	if (nfree == 0 || nfree > nvars_threshold || nfree < relaxdata->options->lag_nvars_to_apply_min) {
		*result = SCIP_DIDNOTRUN;
		return SCIP_OKAY;
	}


	double optimal_value;

	if (relaxdata->options->lag_validate_bounds) {
		SCIP_CALL(solve_submip(scip, &optimal_value));
	}

	if (relaxdata->options->bounds_verbose) {
		cout << "Run: " << relaxdata->nruns << " / Depth: " << SCIPgetDepth(scip) << endl;
	}

	stats.register_name("ddbp_genbound");
	stats.start_timer("ddbp_genbound");

	SCIP_CALL(construct_dd_from_bp_lag(scip, relaxdata->options, relaxdata->output_stats, &dualbound,
		relaxdata->lagrangian_rows, relaxdata->lag_selector));

	stats.end_timer("ddbp_genbound");

	double genbound_time = stats.get_time("ddbp_genbound");

	relaxdata->output_stats->bound_time += genbound_time;

	if (SCIPisStopped(scip)) {
		*result = SCIP_DIDNOTRUN;
		return SCIP_OKAY;
	}

	if (relaxdata->options->bounds_verbose) {
		cout << "Total time: " << genbound_time << endl;
	}

	// Apply bound to SCIP
	SCIP_Real lp_bound = SCIPgetLocalDualbound(scip);
	double external_dualbound = SCIPretransformObj(scip, -dualbound);

	if (relaxdata->options->bounds_verbose) {
		cout.precision(12);
		cout << "LP bound: " << lp_bound << " / Dual bound: " << external_dualbound;
		if (external_dualbound > SCIPceil(scip, lp_bound)) {
			cout << " -- better";
		}
		cout << endl;
	}

	SCIP_Real primal_bound = SCIPgetPrimalbound(scip);

	if (relaxdata->options->bounds_verbose) {
		cout << "Primal bound: " << primal_bound;
		if (primal_bound <= external_dualbound) {
			cout << " -- pruned";
		}
		cout << endl;

		cout << endl;
	}

	// Update output stats
	relaxdata->output_stats->num_runs++;
	if (external_dualbound > SCIPceil(scip, lp_bound)) {
		relaxdata->output_stats->num_runs_improved++;
	}
	if (primal_bound <= external_dualbound) {
		relaxdata->output_stats->num_runs_pruned++;
	}

	if (relaxdata->options->output_stats_verbose) {
		// Initialize to zero if not found (insert does not replace entry)
		auto nvars_num_runs_entry = relaxdata->output_stats->nvars_num_runs.insert(make_pair(nfree, 0));
		nvars_num_runs_entry.first->second++;
		if (external_dualbound > SCIPceil(scip, lp_bound)) {
			auto nvars_num_runs_improved_entry = relaxdata->output_stats->nvars_num_runs_improved.insert(make_pair(nfree, 0));
			nvars_num_runs_improved_entry.first->second++;
		}
		if (primal_bound <= external_dualbound) {
			auto nvars_num_runs_pruned_entry = relaxdata->output_stats->nvars_num_runs_pruned.insert(make_pair(nfree, 0));
			nvars_num_runs_pruned_entry.first->second++;
		}
	}

	if (relaxdata->options->lag_validate_bounds) {
		if (DBL_LT(optimal_value, external_dualbound)) {
			if (relaxdata->options->lag_primal_pruning && DBL_GE_TOL(optimal_value, primal_bound, 1e-6)) { // original tolerance (1e-9) may give false alarms
				cout << "[Validation] Unknown if dual bound is valid (due to primal bound pruning; " << optimal_value << " >= " << primal_bound << ")" << endl;
			} else {
				cout << "[Validation] Warning: Invalid dual bound! -- " << optimal_value << " < " << external_dualbound << endl;
				SCIPABORT();
			}
		} else {
			cout << "[Validation] Dual bound valid: opt " << optimal_value << " >= " << external_dualbound << endl;
		}
	}

	(relaxdata->nruns)++;

	if (relaxdata->options->lag_compute_only) {
		*result = SCIP_DIDNOTRUN; // do nothing with bound
	} else {
		*lowerbound = -dualbound;
		*result = SCIP_SUCCESS;
	}

	if (relaxdata->options->lag_run_once) {
		exit(1);
	}

	return SCIP_OKAY;
}



/*
 * relaxator specific interface methods
 */

/** creates the dd relaxator and includes it in SCIP */
SCIP_RETCODE SCIPincludeRelaxDd(
    SCIP*                 scip,               /**< SCIP data structure */
    Options*              options,            /**< options */
    OutputStats*          output_stats        /**< output statistics */
)
{
	SCIP_RELAXDATA* relaxdata;
	SCIP_RELAX* relax;

	/* create dd relaxator data */
	SCIP_CALL(SCIPallocMemory(scip, &relaxdata));
	SCIP_CALL(SCIPclockCreate(&relaxdata->relaxatorclock, SCIP_CLOCKTYPE_CPU));
	SCIPclockReset(relaxdata->relaxatorclock);
	relaxdata->depthfreq = 1;
	relaxdata->primalatexact = FALSE;
	relaxdata->prunebdd = TRUE;
	relaxdata->options = options;
	relaxdata->output_stats = output_stats;
	relaxdata->nruns = 0;
	relaxdata->disabled = false;
	relaxdata->firstrun = true;

	SCIP_CALL(SCIPclockCreate(&relaxdata->profilingclock, SCIP_CLOCKTYPE_CPU));
	SCIPclockReset(relaxdata->profilingclock);

	relax = NULL;

	int priority = RELAX_PRIORITY;
	if (options->lag_initial_dd) {
		priority = 1000000; // ensures this is called first; priority will be dropped after first call
	}

	/* include relaxator */
	/* use SCIPincludeRelaxBasic() plus setter functions if you want to set callbacks one-by-one and your code should
	 * compile independent of new callbacks being added in future SCIP versions
	 */
	SCIP_CALL(SCIPincludeRelaxBasic(scip, &relax, RELAX_NAME, RELAX_DESC, priority, RELAX_FREQ,
	                                relaxExecDd, relaxdata));

	assert(relax != NULL);

	/* set non fundamental callbacks via setter functions */
	SCIP_CALL(SCIPsetRelaxCopy(scip, relax, relaxCopyDd));
	SCIP_CALL(SCIPsetRelaxFree(scip, relax, relaxFreeDd));
	SCIP_CALL(SCIPsetRelaxInit(scip, relax, relaxInitDd));
	SCIP_CALL(SCIPsetRelaxExit(scip, relax, relaxExitDd));
	SCIP_CALL(SCIPsetRelaxInitsol(scip, relax, relaxInitsolDd));
	SCIP_CALL(SCIPsetRelaxExitsol(scip, relax, relaxExitsolDd));

	return SCIP_OKAY;
}
