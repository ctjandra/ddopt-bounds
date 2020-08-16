#include "lg_dd_selector_scip.hpp"

#include "../problem/bp/prop_linearcons.hpp"
#include "../problem/bp/bp_reader_scip.hpp"
#include "../core/mergers.hpp"

vector<int> get_fixed_variables_binary_scip(SCIP* scip, SCIP_COL** cols, int ncols)
{
	vector<int> fixed_vars;

	fixed_vars.resize(ncols);
	for (int j = 0; j < ncols; ++j) {
		SCIP_VAR* scip_var = SCIPcolGetVar(cols[j]);
		assert(SCIPvarIsBinary(scip_var)); // Assume binary
		int idx = SCIPvarGetProbindex(scip_var);
		assert(idx >= 0 && idx < ncols);
		if (SCIPisEQ(scip, SCIPvarGetLbLocal(scip_var), 0.0) && SCIPisEQ(scip, SCIPvarGetUbLocal(scip_var), 0.0)) {
			fixed_vars[idx] = 0;
		} else if (SCIPisEQ(scip, SCIPvarGetLbLocal(scip_var), 1.0) && SCIPisEQ(scip, SCIPvarGetUbLocal(scip_var), 1.0)) {
			fixed_vars[idx] = 1;
		} else {
			fixed_vars[idx] = DD_UNFIXED_VAR; // Assumes variables are nonnegative
		}
	}

	return fixed_vars;
}


void LagrangianDDConstraintSelector::create_mappings_default(SCIP* scip, const vector<int>& fixed_vars,
        vector<int>& var_to_subvar, vector<int>& subvar_to_var)
{
	SCIP_COL** cols;
	int ncols;

	// Assume SCIP is mid-solve
	assert(SCIPgetStage(scip) == SCIP_STAGE_SOLVING);
	SCIP_CALL_ABORT(SCIPgetLPColsData(scip, &cols, &ncols));

	int subvar = 0;
	subvar_to_var.clear();
	var_to_subvar.resize(ncols, -1);
	for (int i = 0; i < ncols; ++i) {
		int idx = SCIPvarGetProbindex(SCIPcolGetVar(cols[i]));
		if (fixed_vars[idx] == DD_UNFIXED_VAR) {
			var_to_subvar[idx] = subvar;
			subvar_to_var.push_back(idx);
			subvar++;
		}
	}
}