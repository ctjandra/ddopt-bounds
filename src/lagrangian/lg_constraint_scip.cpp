#include "lg_constraint_scip.hpp"
#include "lg_dd_selector_scip.hpp"


/**
 * Extract rows from SCIP into a vector of Lagrangian constraints. Skips rows marked in row_skip_lhs and row_skip_rhs.
 * Also takes a vector of fixed variables and adjusts RHSs; if empty, uses information from SCIP to find out fixed variables.
 */
vector<LagrangianConstraint> lagrangian_extract_constraints_scip(SCIP* scip, SCIP_ROW** rows, int nrows,
        const vector<bool>& row_skip_lhs, const vector<bool>& row_skip_rhs, bool clean_up_assume_binary,
        const vector<int>& fixed_vars /* = {} */)
{

	// Warning: This may be dangerous to use because SCIP might delete rows throughout solving. This might be ok to use if
	// presolve is off. See the other version below uses SCIPRowVector*, which maintains copies of rows. Kept for simpler
	// tests and backwards compatibility.

	vector<LagrangianConstraint> relaxed_constrs;
	vector<int> ind;
	vector<double> coeffs;

	// fixed_vars is either empty, in which case info from SCIP is used, or corresponds to number of variables
	assert(fixed_vars.empty() || (int) fixed_vars.size() == SCIPgetNLPCols(scip));
	bool retrieve_fixed_vars_from_scip = (fixed_vars.empty());

	for (int i = 0; i < nrows; ++i) {
		SCIP_ROW* row = rows[i];
		SCIP_Real lhs = SCIProwGetLhs(row);
		SCIP_Real rhs = SCIProwGetRhs(row);

		bool skip_lhs = (row_skip_lhs[i] || SCIPisInfinity(scip, -lhs));
		bool skip_rhs = (row_skip_rhs[i] || SCIPisInfinity(scip, rhs));

		if (skip_lhs && skip_rhs) {
			continue;
		}

		if (!skip_lhs) {
			lhs -= SCIProwGetConstant(row);
		}
		if (!skip_rhs) {
			rhs -= SCIProwGetConstant(row);
		}

		ind.clear();
		coeffs.clear();

		SCIP_Real* row_vals = SCIProwGetVals(row);
		SCIP_COL** row_cols = SCIProwGetCols(row);

		int nnonz = SCIProwGetNNonz(row);
		for (int j = 0; j < nnonz; ++j) {
			SCIP_VAR* var = SCIPcolGetVar(row_cols[j]);
			int idx = SCIPvarGetProbindex(var);

			bool var_is_fixed = false;
			SCIP_Real fixed_val = 0;

			// Retrieve from either SCIP or from the given vector whether variable is fixed and its value
			if (retrieve_fixed_vars_from_scip) {
				SCIP_Real var_lb = SCIPvarGetLbLocal(var);
				SCIP_Real var_ub = SCIPvarGetUbLocal(var);
				var_is_fixed = (SCIPisEQ(scip, var_lb, var_ub));
				fixed_val = var_lb;
			} else {
				assert(!fixed_vars.empty());
				fixed_val = fixed_vars[idx];
				var_is_fixed = (fixed_val != DD_UNFIXED_VAR);
			}

			// Add variables that are not fixed to LagrangianConstraint or update RHS for those that are
			if (!var_is_fixed) {
				coeffs.push_back(row_vals[j]);
				ind.push_back(idx);
				assert(idx < SCIPgetNLPCols(scip));
			} else {
				// i-th variable is fixed to fixed_val
				if (!skip_lhs) {
					lhs -= row_vals[j] * fixed_val;
				}
				if (!skip_rhs) {
					rhs -= row_vals[j] * fixed_val;
				}
			}
		}

		if (clean_up_assume_binary) {
			// Clean up redundant rows (assume variables are binary)
			if (!skip_lhs) { // >= type constraint
				double minactivity = 0;
				int nnonz_cons = coeffs.size();
				for (int j = 0; j < nnonz_cons; ++j) {
					minactivity += MIN(0, coeffs[j]);
				}
				if (DBL_GE(minactivity, lhs)) {
					skip_lhs = true;
				}
			}
			if (!skip_rhs) { // >= type constraint
				double maxactivity = 0;
				int nnonz_cons = coeffs.size();
				for (int j = 0; j < nnonz_cons; ++j) {
					maxactivity += MAX(0, coeffs[j]);
				}
				if (DBL_LE(maxactivity, rhs)) {
					skip_rhs = true;
				}
			}
		}

		if (coeffs.size() > 0) {
			if (!skip_lhs) {
				relaxed_constrs.push_back(LagrangianConstraint(ind, coeffs, lhs, LINSENSE_GE));
			}

			if (!skip_rhs) {
				relaxed_constrs.push_back(LagrangianConstraint(ind, coeffs, rhs, LINSENSE_LE));
			}
		}
	}

	return relaxed_constrs;
}


// Note: This is essentially a copy of lagrangian_extract_constraints_scip above, but with copies of SCIP_ROW* instead of using
// the original ones, which might be removed by SCIP.
// clean_up_assume_binary assumes all variables are binary and removes rows that are redundant to binary bounds
vector<LagrangianConstraint> lagrangian_extract_constraints_scip(SCIP* scip, SCIPRowVector* lagrangian_rows,
        bool clean_up_assume_binary, const vector<int>& fixed_vars)
{
	vector<bool> row_skip_lhs(lagrangian_rows->lhs);
	vector<bool> row_skip_rhs(lagrangian_rows->rhs);
	row_skip_lhs.flip();
	row_skip_rhs.flip();
	int nrows = lagrangian_rows->rows.size();

	vector<LagrangianConstraint> relaxed_constrs;
	vector<int> ind;
	vector<double> coeffs;

	assert((int) fixed_vars.size() == SCIPgetNLPCols(scip));

	for (int i = 0; i < nrows; ++i) {
		SCIPRow* row = lagrangian_rows->rows[i];
		SCIP_Real lhs = row->lhs;
		SCIP_Real rhs = row->rhs;
		SCIP_Real constant = row->constant;

		bool skip_lhs = (row_skip_lhs[i] || SCIPisInfinity(scip, -lhs));
		bool skip_rhs = (row_skip_rhs[i] || SCIPisInfinity(scip, rhs));

		if (skip_lhs && skip_rhs) {
			continue;
		}

		if (!skip_lhs) {
			lhs -= constant;
		}
		if (!skip_rhs) {
			rhs -= constant;
		}

		ind.clear();
		coeffs.clear();

		int nnonz = row->nnonz;
		for (int j = 0; j < nnonz; ++j) {
			int idx = row->col_ids[j];

			bool var_is_fixed = false;
			SCIP_Real fixed_val = 0;

			fixed_val = fixed_vars[idx];
			var_is_fixed = (fixed_val != DD_UNFIXED_VAR);

			// Add variables that are not fixed to LagrangianConstraint or update RHS for those that are
			if (!var_is_fixed) {
				coeffs.push_back(row->vals[j]);
				ind.push_back(idx);
				assert(idx < SCIPgetNLPCols(scip));
			} else {
				// i-th variable is fixed to fixed_val
				if (!skip_lhs) {
					lhs -= row->vals[j] * fixed_val;
				}
				if (!skip_rhs) {
					rhs -= row->vals[j] * fixed_val;
				}
			}
		}

		if (clean_up_assume_binary) {
			// Clean up redundant rows (assume variables are binary)
			if (!skip_lhs) { // >= type constraint
				double minactivity = 0;
				int nnonz_cons = coeffs.size();
				for (int j = 0; j < nnonz_cons; ++j) {
					minactivity += MIN(0, coeffs[j]);
				}
				if (DBL_GE(minactivity, lhs)) {
					skip_lhs = true;
					// cout << "Skipped: ";
					// for (int j = 0; j < nnonz_cons; ++j) {
					// 	cout << " + " << coeffs[j] << " x" << ind[j];
					// }
					// cout << " >= " << lhs << endl;
				}
			}
			if (!skip_rhs) { // >= type constraint
				double maxactivity = 0;
				int nnonz_cons = coeffs.size();
				for (int j = 0; j < nnonz_cons; ++j) {
					maxactivity += MAX(0, coeffs[j]);
				}
				if (DBL_LE(maxactivity, rhs)) {
					skip_rhs = true;
					// cout << "Skipped: ";
					// for (int j = 0; j < nnonz_cons; ++j) {
					// 	cout << " + " << coeffs[j] << " x" << ind[j];
					// }
					// cout << " <= " << rhs << endl;
				}
			}
		}

		if (coeffs.size() > 0) {
			if (!skip_lhs) {
				relaxed_constrs.push_back(LagrangianConstraint(ind, coeffs, lhs, LINSENSE_GE));
			}

			if (!skip_rhs) {
				relaxed_constrs.push_back(LagrangianConstraint(ind, coeffs, rhs, LINSENSE_LE));
			}
		}
	}

	return relaxed_constrs;
}


vector<LagrangianConstraint> lagrangian_extract_all_constraints_scip(SCIP* scip, SCIP_ROW** rows, int nrows)
{
	return lagrangian_extract_constraints_scip(scip, rows, nrows, vector<bool>(nrows, false), vector<bool>(nrows, false), false);
}
