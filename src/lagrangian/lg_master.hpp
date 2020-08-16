/* Master problem for Lagrangian relaxation: abstract class */

#ifndef LG_MASTER_HPP_
#define LG_MASTER_HPP_

#include <vector>
#include "lg_constraint.hpp"

using namespace std;

class LagrangianMasterProblem
{
protected:
	vector<LagrangianConstraint> relaxed_constrs;

	double optval;                    /**< optimal value from the last solve */


public:
	LagrangianMasterProblem(const vector<LagrangianConstraint>& _relaxed_constrs) : relaxed_constrs(_relaxed_constrs) {}

	virtual ~LagrangianMasterProblem() {}


	/** Update the master problem with the subproblem optimal solution and value */
	virtual void update(const vector<int>& sp_optsol, double sp_optval, const vector<double>& lambdas) = 0;

	/**
	 * Solve the master problem, returning the objective and the optimal Lagrange multipliers
	 * at this iteration
	 */
	virtual double solve(vector<double>& lambdas) = 0;

	/** Store in primal_sol the primal solution recovered from the master problem and return its value */
	virtual double recover_primal_sol(vector<double>& primal_sol) = 0;

	int get_nrows_lag()
	{
		return relaxed_constrs.size();
	}


};


#endif // LG_MASTER_HPP_