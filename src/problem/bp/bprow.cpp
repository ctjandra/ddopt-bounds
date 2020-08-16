/**
 * Row for binary problem
 */

#include <algorithm>
#include <iterator>
#include "bprow.hpp"
#include "bpvar.hpp"


#ifdef SOLVER_SCIP

BPRow::BPRow(SCIP* scip, SCIP_ROW* row, RowSense _sense, const vector<BPVar*>& free_vars, const char* _type, bool consider_fixed)
{
	sense = _sense;
	type = _type;
	rhs = (sense == SENSE_LE) ? SCIProwGetRhs(row) : SCIProwGetLhs(row);
	rhs -= SCIProwGetConstant(row);

	SCIP_Real* vals = SCIProwGetVals(row);
	SCIP_COL** cols = SCIProwGetCols(row);
	nnonz = 0;
	int nnonz_all = SCIProwGetNNonz(row);
	for (int i = 0; i < nnonz_all; ++i) {
		SCIP_VAR* var = cols[i]->var;
		SCIP_Real var_lb = SCIPvarGetLbLocal(var);
		SCIP_Real var_ub = SCIPvarGetUbLocal(var);

		/* take variables that are not fixed or update rhs for those that are */
		if (!consider_fixed || !SCIPisEQ(scip, var_lb, var_ub)) {
			int cur_ind = -1;
			int nfree_vars = free_vars.size();

			for (int j = 0; j < nfree_vars; ++j) {
				if (free_vars[j]->solver_index == SCIPvarGetProbindex(var)) {
					assert(free_vars[j]->index == j);
					cur_ind = free_vars[j]->index;
					break;
				}
			}
			/* variable must be in the array of unfixed variables */
			assert(cur_ind >= 0);

			coeffs.push_back(vals[i]);
			ind.push_back(cur_ind);
			nnonz++;
		} else { /* i-th variable is fixed to var_lb */
			rhs -= vals[i] * var_lb;
		}
	}

	assert(nnonz == (int) ind.size());
	assert(nnonz == (int) coeffs.size());
}

#endif

double BPRow::calculate_minactivity()
{
	double minactivity = 0;
	for (double coeff : coeffs) {
		if (coeff < 0) {
			minactivity += coeff;
		}
	}
	return minactivity;
}

double BPRow::calculate_maxactivity()
{
	double maxactivity = 0;
	for (double coeff : coeffs) {
		if (coeff > 0) {
			maxactivity += coeff;
		}
	}
	return maxactivity;
}

double BPRow::get_coeff(int var)
{
	vector<int>::iterator it = std::find(ind.begin(), ind.end(), var);
	if (it == ind.end()) { // not found
		return 0;
	}
	int i = std::distance(ind.begin(), it);
	return coeffs[i];
}

ostream& operator<<(std::ostream& os, const BPRow& row)
{
	for (int i = 0; i < row.nnonz; ++i) {
		os << " + " << row.coeffs[i] << "<x" << row.ind[i] << ">";
	}
	if (row.sense == SENSE_LE) {
		os << " <= ";
	} else {
		os << " >= ";
	}
	os << row.rhs << " ";
	return os;
}

void BPRow::print_nonzero_coeffs(int nvars)
{
	vector<int> nonzero_coeffs(nvars, 0);
	for (int i = 0; i < nnonz; ++i) {
		if (!DBL_EQ(coeffs[i], 0)) {
			assert(ind[i] < nvars);
			nonzero_coeffs[ind[i]] = 1;
		}
	}

	for (int i = 0; i < nvars; ++i) {
		if (nonzero_coeffs[i] == 0) {
			cout << " ";
		} else {
			cout << "*";
		}
	}
	cout << endl;
}
