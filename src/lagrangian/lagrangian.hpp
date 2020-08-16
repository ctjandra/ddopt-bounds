#ifndef LAGRANGIAN_HPP_
#define LAGRANGIAN_HPP_

#include <vector>
#include "lg_master.hpp"
#include "lg_subprob.hpp"


struct LagrangianRelaxationParams {
	double convergence_tol  = 1e-4;          /**< tolerance for optimality check (|dual - primal| <= tolerance) */
	int max_niters          = -1;            /**< maximum number of iterations for the Lagrangian relaxation (-1: no limit) */
	int max_noracleiters    = -1;            /**< maximum number of oracle calls for the Lagrangian relaxation (-1: no limit) */
	double time_limit       = -1;            /**< time limit */
	double obj_limit        = -numeric_limits<double>::infinity();   /**< objective limit (only implemented for CB) */
};


class LagrangianRelaxation
{
private:

	LagrangianMasterProblem* master;             /**< master problem for the Lagrangian relaxation */
	LagrangianSubproblem*    subproblem;         /**< subproblem for the Lagrangian relaxation */

public:

	LagrangianRelaxation(LagrangianMasterProblem* _master, LagrangianSubproblem* _subproblem) :
		master(_master), subproblem(_subproblem) {}

	~LagrangianRelaxation() {}

	void solve(LagrangianRelaxationParams params);
};


#endif // LAGRANGIAN_HPP_