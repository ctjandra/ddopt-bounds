/* Subproblem for Lagrangian relaxation: interface */

#ifndef LG_SUBPROB_HPP_
#define LG_SUBPROB_HPP_

#include <vector>
#include <cassert>
#include "../util/util.hpp"

using namespace std;

class LagrangianSubproblem
{
public:

	virtual ~LagrangianSubproblem() {}

	/**
	 * Solve a subproblem for the Lagrangian relaxation with multipliers lambdas.
	 * Store the optimal solution in optsol and return the optimal value.
	 */
	virtual double solve(const vector<double>& lambdas, vector<int>& optsol) = 0;

};

/**
 * Oracle for a Lagrangian subproblem. Solve a single problem, without concerning about Lagrange multipliers,
 * decomposition, and variable mapping, which are handled in LagrangianSubproblem.
 */
class LagrangianSubproblemOracle
{
public:

	virtual ~LagrangianSubproblemOracle() {}

	/**
	 * Solve a subproblem for the Lagrangian relaxation with objective obj.
	 * Store the optimal solution in optsol and return the optimal value.
	 */
	virtual double solve(const vector<double>& obj, vector<int>& optsol) = 0;

protected:

	/** Return value of a solution. Convenience function that may be used returning the objective value in solve. */
	double get_optimal_value(const vector<double>& obj, const vector<int>& optsol);
};


/** Simple oracle that returns the optimal solution in the 0-1 cube. */
class LagrangianSubproblemOracleZeroOne : public LagrangianSubproblemOracle
{
public:

	/** Calculate optimal solution in 0-1 cube. */
	double solve(const vector<double>& obj, vector<int>& optsol)
	{
		int nvars = obj.size();
		optsol.resize(nvars);
		for (int i = 0; i < nvars; ++i) {
			if (DBL_GT(obj[i], 0)) {
				optsol[i] = 1;
			} else {
				optsol[i] = 0;
			}
		}
		return get_optimal_value(obj, optsol);
	}
};


inline double LagrangianSubproblemOracle::get_optimal_value(const vector<double>& obj, const vector<int>& optsol)
{
	int nvars = obj.size();
	assert(nvars == (int) optsol.size());
	double optval = 0;
	for (int i = 0; i < nvars; ++i) {
		optval += obj[i] * optsol[i];
	}
	return optval;
}

#endif // LG_SUBPROB_HPP_
