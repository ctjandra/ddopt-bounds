#include "cliquetable_cons_id.hpp"

void is_row_clique_table_form(SCIP* scip, SCIP_ROW* row, bool* ret_lhs, bool* ret_rhs)
{
	// Initialize output
	*ret_rhs = true;
	*ret_lhs = true;

	SCIP_Real rhs = SCIProwGetRhs(row);
	SCIP_Real lhs = SCIProwGetLhs(row);

	if (!SCIPisInfinity(scip, -lhs)) {
		lhs -= SCIProwGetConstant(row);
	}
	if (!SCIPisInfinity(scip, rhs)) {
		rhs -= SCIProwGetConstant(row);
	}

	SCIP_Real* vals = SCIProwGetVals(row);
	int nnonz = SCIProwGetNNonz(row);
	if (nnonz == 0) {
		// true if valid, false if not
		*ret_rhs = SCIPisGE(scip, rhs, 0.0);
		*ret_lhs = SCIPisLE(scip, lhs, 0.0);
		return;
	}

	SCIP_Real coeff = abs(vals[0]);
	int npos = 0;
	int nneg = 0;

	// Check if all coefficients are the same in absolute value and count them according to sign
	for (int i = 0; i < nnonz; ++i) {
		if (SCIPisEQ(scip, vals[i], coeff)) { // positive coefficient
			npos++;
		} else if (SCIPisEQ(scip, vals[i], -coeff)) { // negative coefficient
			nneg++;
		} else {
			// Different coefficients; not in clique table form
			*ret_rhs = false;
			*ret_lhs = false;
			return;
		}
	}
	assert(npos + nneg == nnonz);

	// Check lhs/rhs
	if (!SCIPisEQ(scip, rhs, 1 - nneg)) {
		*ret_rhs = false;
	}
	if (!SCIPisEQ(scip, lhs, npos - 1)) {
		*ret_lhs = false;
	}
}


bool check_clique_table_form_consistent(SCIP* scip, SCIP_ROW* row)
{
	bool lhs_ctform;
	bool rhs_ctform;
	is_row_clique_table_form(scip, row, &lhs_ctform, &rhs_ctform);

	if (!lhs_ctform && !rhs_ctform) {
		return true; // nothing to check
	}

	SCIP_COL** cols = SCIProwGetCols(row);
	SCIP_Real* vals = SCIProwGetVals(row);
	int nnonz = SCIProwGetNNonz(row);
	if (nnonz == 0) {
		return true;
	}

	// Each pair of (signed) variables must be adjacent in some clique (not necessarily the same for all)
	// This is a slow check, but ok since this is used only for testing
	for (int i = 0; i < nnonz; ++i) {
		for (int j = i + 1; j < nnonz; ++j) {
			SCIP_VAR* var_i = SCIPcolGetVar(cols[i]);
			SCIP_VAR* var_j = SCIPcolGetVar(cols[j]);
			SCIP_Bool sign_i = SCIPisGE(scip, vals[i], 0.0);
			SCIP_Bool sign_j = SCIPisGE(scip, vals[j], 0.0);

			// Return false if variables should be adjacent but they are not
			if (rhs_ctform && !SCIPhaveVarsCommonClique(scip, var_i, sign_i, var_j, sign_j, FALSE)) {
				std::cout << sign_i << " " << SCIPvarGetName(var_i) << ", " << sign_j << " " << SCIPvarGetName(var_j) << std::endl;
				return false;
			}
			if (lhs_ctform && !SCIPhaveVarsCommonClique(scip, var_i, !sign_i, var_j, !sign_j, FALSE)) {
				std::cout << !sign_i << " " << SCIPvarGetName(var_i) << ", " << !sign_j << " " << SCIPvarGetName(var_j) << std::endl;
				return false;
			}
		}
	}

	return true;
}
