/** Strategy for decision diagram + Lagrangian relaxation bounds */

#ifndef LG_DD_SELECTOR_SCIP_HPP_
#define LG_DD_SELECTOR_SCIP_HPP_

#include "scip/scip.h"
#include "../core/solver.hpp"
#include "../util/graph.hpp"
#include "lg_constraint_scip.hpp"

#define DD_UNFIXED_VAR -1  // Must be negative number (assumes variables are nonnegative)


/** Describes how to select constraints for decision diagrams and for Lagrangian relaxation */
class LagrangianDDConstraintSelector
{
protected:

	/** Default implementation for create_mappings: create directly from fixed_vars */
	void create_mappings_default(SCIP* scip, const vector<int>& fixed_vars, vector<int>& var_to_subvar,
	                             vector<int>& subvar_to_var);

public:
	virtual ~LagrangianDDConstraintSelector() {}

	/** Return true if there is some structure to exploit and this approach can be used */
	virtual bool exists_structure(SCIP* scip, SCIP_ROW** rows, int nrows) = 0;

	/** Extract rows that will be Lagrangianized for the bound.  Called at the beginning of solving. */
	virtual SCIPRowVector* extract_lagrangian_rows(SCIP* scip, SCIP_ROW** rows, int nrows, Options* options) = 0;

	/** Prepare necessary structures for DD construction. Called at the beginning of solving. */
	virtual void prepare_dd_construction(SCIP* scip, SCIP_ROW** rows, int nrows, Options* options) = 0;

	/**
	 * Run some preprocessing in order to expand the set of fixed variables. The given fixed_vars should be the ones fixed
	 * by the solver, and this function may or may not add more fixed variables to this vector. The fixed variables must be
	 * valid within the scope of the entire (restricted) subproblem, not just the DD constraints.
	 * Fixed_vars are indexed by problem index and negative values means variables are not fixed (assumes nonnegative).
	 */
	virtual void preprocess_fixed_vars(SCIP* scip, vector<int>& fixed_vars) = 0;

	/**
	 * Create mappings between subspace and original space for DD construction. The subspace should exclude the given fixed
	 * variables and may exclude even further variables that are not fixed but not present in any constraint relevant for the DD.
	 */
	virtual void create_mappings(SCIP* scip, const vector<int>& fixed_vars, vector<int>& var_to_subvar,
	                             vector<int>& subvar_to_var) = 0;

	/** Create decision diagram solver for problem */
	virtual DDSolver* create_solver(SCIP* scip, const vector<int>& var_to_subvar, const vector<int>& subvar_to_var,
	                                const vector<int>& fixed_vars, const vector<double>& obj, Options* options) = 0;
};


/** Create vector indexed by probindex of variables with the value they are fixed to, or -1 if not fixed */
vector<int> get_fixed_variables_binary_scip(SCIP* scip, SCIP_COL** cols, int ncols);


#endif // LG_DD_SELECTOR_SCIP_HPP_