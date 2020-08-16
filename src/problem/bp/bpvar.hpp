/**
 * Variable for binary problem
 */

#ifndef BPVAR_HPP_
#define BPVAR_HPP_

#ifdef SOLVER_SCIP
#include "scip/scip.h"
#include "scip/var.h"
#include "scip/lp.h"
#endif

#include <vector>
#include <string>

using namespace std;

struct BPRow; /* forward declaration */

/**
 * Variable
 */
struct BPVar {
	double obj;                /**< objective value for variable */
	vector<int> rows;          /**< row indices in which this variable has a nonzero coefficient */
	vector<double> row_coeffs; /**< row coefficients for each row in rows */
	int index;                 /**< unique identifier for this variable; will be used in ind in BPRows */

	int solver_index;          /**< solver's index of var (used only at initialization of rows/vars) */

#ifdef SOLVER_SCIP
	/** Constructor from SCIP var (does not set rows; use init_rows) */
	BPVar(SCIP_VAR* var, int _index);
#endif


	/** Manual constructor (does not set rows; use init_rows) */
	BPVar(double _obj, int _index) : obj(_obj), index(_index), solver_index(_index) {}

	/** Copy BPVar */
	BPVar(BPVar* var, bool copy_rows = true)
	{
		obj = var->obj;
		index = var->index;
		solver_index = var->solver_index;
		if (copy_rows) {
			rows = var->rows;
			row_coeffs = var->row_coeffs;
		}
	}

	/** Initialize rows and row_coeffs based on given rows */
	void init_rows(const vector<BPRow*>& all_rows);
};

#endif /* BPVAR_HPP_ */
