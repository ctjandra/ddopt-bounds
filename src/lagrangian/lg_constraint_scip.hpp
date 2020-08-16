#ifndef LG_CONSTRAINT_SCIP_HPP_
#define LG_CONSTRAINT_SCIP_HPP_

#include <vector>
#include "scip/scip.h"
#include "scip/pub_lp.h"
#include "lg_constraint.hpp"


/** Copy of SCIP_ROW; used because SCIP might remove rows during the solving process */
struct SCIPRow {
	SCIP_Real lhs;
	SCIP_Real rhs;
	SCIP_Real constant;
	vector<SCIP_Real> vals;
	vector<int> col_ids;
	int nnonz;

	SCIPRow(SCIP_ROW* row) : lhs(SCIProwGetLhs(row)), rhs(SCIProwGetRhs(row)), constant(SCIProwGetConstant(row))
	{
		SCIP_Real* row_vals = SCIProwGetVals(row);
		SCIP_COL** row_cols = SCIProwGetCols(row);
		nnonz = SCIProwGetNNonz(row);
		vals.insert(vals.end(), &row_vals[0], &row_vals[nnonz]);
		col_ids.reserve(nnonz);
		for (int j = 0; j < nnonz; ++j) {
			SCIP_VAR* var = SCIPcolGetVar(row_cols[j]);
			col_ids.push_back(SCIPvarGetProbindex(var));
		}
	}
};


/** Vector of SCIP_ROWs with booleans indicating whether RHS or LHS is to be considered as part of the set */
struct SCIPRowVector {
	vector<SCIPRow*> rows;
	vector<bool> lhs; // if lhs[i] is true, the left-hand inequality (>=) of rows[i] is meant to be part of the list
	vector<bool> rhs; // if rhs[i] is true, the right-hand inequality (<=) of rows[i] is meant to be part of the list

	~SCIPRowVector()
	{
		for (SCIPRow* row : rows) {
			delete row; // these are only created by this class
		}
	}

	/** Add a new row. Do nothing if both row_lhs and row_rhs are false */
	void push_back(SCIP_ROW* row, bool row_lhs, bool row_rhs)
	{
		assert(rows.size() == lhs.size());
		assert(rows.size() == rhs.size());
		if (row_lhs || row_rhs) {
			rows.push_back(new SCIPRow(row));
			lhs.push_back(row_lhs);
			rhs.push_back(row_rhs);
		}
	}

	int size()
	{
		return rows.size();
	}
};



/**
 * Extract rows from SCIP into a vector of Lagrangian constraints. Skips rows marked in row_skip_lhs and row_skip_rhs.
 * Also takes into account a vector of fixed variables; if empty, uses information from SCIP to find out fixed variables.
 */
vector<LagrangianConstraint> lagrangian_extract_constraints_scip(SCIP* scip, SCIP_ROW** rows, int nrows,
        const vector<bool>& row_skip_lhs, const vector<bool>& row_skip_rhs, bool clean_up_assume_binary,
        const vector<int>& fixed_vars = {});

/** Extract all rows from SCIP into a vector of Lagrangian constraints. */
vector<LagrangianConstraint> lagrangian_extract_all_constraints_scip(SCIP* scip, SCIP_ROW** rows, int nrows);

/**
 * Convert rows from lagrangian_rows into a vector of Lagrangian constraints. Also takes into account a vector of fixed
 * variables; if empty, uses information from SCIP to find out fixed variables.
 */
vector<LagrangianConstraint> lagrangian_extract_constraints_scip(SCIP* scip, SCIPRowVector* lagrangian_rows,
        bool clean_up_assume_binary, const vector<int>& fixed_vars);

#endif // LG_CONSTRAINT_SCIP_HPP_