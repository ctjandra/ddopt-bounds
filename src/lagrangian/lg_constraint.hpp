/** Constraint for Lagrangian relaxation */

#ifndef LG_CONSTRAINT_HPP_
#define LG_CONSTRAINT_HPP_

#include <vector>
#include <cassert>
#include <iostream>

using namespace std;

/** Sense for linear constraints; includes equality */
enum LinSense {
	LINSENSE_LE,   /* <= */
	LINSENSE_GE,   /* >= */
	LINSENSE_EQ    /* == */
};


/** Linear constraint relaxed in Lagrangian relaxation */
struct LagrangianConstraint {
	vector<int> ind;             /**< support of constraint */
	vector<double> coeffs;       /**< coeffs[i] is the coefficient of variable ind[i] */
	int nnonz;                   /**< number of nonzeros; also size of ind and coeffs */
	double rhs;                  /**< right-hand side of constraint */
	LinSense sense;              /**< sense for constraint (<=, >=, ==) */


	LagrangianConstraint(const vector<int>& _ind, const vector<double>& _coeffs, double _rhs, LinSense _sense) :
		ind(_ind), coeffs(_coeffs), rhs(_rhs), sense(_sense)
	{
		assert(ind.size() == coeffs.size());
		nnonz = ind.size();
	}

	~LagrangianConstraint() {}

	/** Return a subgradient of the Lagrangian dual w.r.t. this constraint at a given point x */
	double get_lagrangian_subgradient(const vector<int>& x)
	{
		double slack = rhs;
		for (int i = 0; i < nnonz; ++i) {
			assert(ind[i] < (int) x.size());
			slack -= coeffs[i] * x[ind[i]];
		}
		return slack;
	}

	friend ostream& operator<<(ostream& os, const LagrangianConstraint& constr);
};

inline ostream& operator<<(ostream& os, const LagrangianConstraint& constr)
{
	for (int j = 0; j < constr.nnonz; ++j) {
		os << " + " << constr.coeffs[j] << " x" << constr.ind[j];
	}
	if (constr.sense == LINSENSE_LE) {
		os << " <= ";
	} else if (constr.sense == LINSENSE_GE) {
		os << " >= ";
	} else {
		os << " == ";
	}
	os << constr.rhs;
	return os;
}


#endif // LG_CONSTRAINT_HPP_
