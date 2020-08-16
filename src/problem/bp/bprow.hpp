/**
 * Row for binary problem
 */

#ifndef BPROW_HPP_
#define BPROW_HPP_

#include "bpvar.hpp"

#ifdef SOLVER_SCIP
#include "scip/scip.h"
#endif

#include <vector>
#include <iostream>

#include "../../util/util.hpp"

using namespace std;

enum RowSense {
	SENSE_LE = 0,
	SENSE_GE
};

/**
 * Row representing a constraint (with only the relevant variables, unlike the solver's row structure)
 */
struct BPRow {
	double rhs;              /**< right-hand side of constraint */
	RowSense sense;          /**< sense of constraint (<= or >=) */
	vector<double> coeffs;   /**< coefficients of the constraint */
	vector<int> ind;         /**< variable indices of the constraint (w.r.t. a given array at construction) */
	int nnonz;               /**< number of coefficients/variables in the constraint */
	const char* type;        /**< type of constraint */

#ifdef SOLVER_SCIP
	/** Constructor from SCIP row (indexing is relative to free_vars array) */
	BPRow(SCIP* scip, SCIP_ROW* row, RowSense _sense, const vector<BPVar*>& free_vars, const char* type, bool consider_fixed=true);
#endif

	/** Manual constructor */
	BPRow(double _rhs, RowSense _sense, const vector<double>& _coeffs, const vector<int>& _ind) :
		rhs(_rhs), sense(_sense), coeffs(_coeffs), ind(_ind)
	{
		if (coeffs.size() != ind.size()) {
			cout << "Error: Row with incompatible coefficients and indices dimensions" << endl;
			exit(1);
		}
		nnonz = coeffs.size();
		type = NULL;
	}

	/** Create copy of row with only variable indices in subspace */
	BPRow(BPRow* row, const vector<bool>& subspace)
	{
		rhs = row->rhs;
		sense = row->sense;
		type = row->type;
		ind.reserve(row->nnonz);
		coeffs.reserve(row->nnonz);
		for (int i = 0; i < row->nnonz; ++i) {
			if (subspace[row->ind[i]]) {
				ind.push_back(row->ind[i]);
				coeffs.push_back(row->coeffs[i]);
			}
		}
		nnonz = coeffs.size();
	}

	/** Create copy of row with certain variables fixed to values (-1 means unfixed) */
	BPRow(BPRow* row, const vector<int>& fixed_vars)
	{
		rhs = row->rhs;
		sense = row->sense;
		type = row->type;
		ind.reserve(row->nnonz);
		coeffs.reserve(row->nnonz);
		for (int i = 0; i < row->nnonz; ++i) {
			int val = fixed_vars[row->ind[i]];
			if (val == -1) {
				ind.push_back(row->ind[i]);
				coeffs.push_back(row->coeffs[i]);
			} else {
				assert(val == 0 || val == 1);
				rhs -= val * row->coeffs[i];
			}
		}
		nnonz = coeffs.size();
	}

	/** Calculate the minimum left-hand side value given that variables are binary */
	double calculate_minactivity();

	/** Calculate the maximum left-hand side value given that variables are binary */
	double calculate_maxactivity();

	/** Returns the coefficient of the given variable. Linear search. */
	double get_coeff(int var);

	/** Prints row */
	friend ostream& operator<<(std::ostream& os, const BPRow& row);

	/** Print " " for zero coefficients and "*" for nonzero coefficients; used to visualize sparsity of coefficient matrix */
	void print_nonzero_coeffs(int nvars);
};

#endif /* BPROW_HPP_ */
