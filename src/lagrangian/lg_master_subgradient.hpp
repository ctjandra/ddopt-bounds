/* Master problem for Lagrangian relaxation: Simple subgradient method */

#ifndef LG_MASTER_SUBGRADIENT_HPP_
#define LG_MASTER_SUBGRADIENT_HPP_

#include <limits>
#include <iostream>
#include "lg_master.hpp"
#include "../util/util.hpp"


class LagrangianMasterProblemSubgradient : public LagrangianMasterProblem
{
private:
	vector<double> current_subgradients;
	double stepsize;
	double stepscale;
	int nstalls;
	double best_lag_dual;
	int ncalls;

	/** Return step size for subgradient method; current values must be up-to-date */
	double compute_stepsize(double current_lag_dual);

public:
	LagrangianMasterProblemSubgradient(const vector<LagrangianConstraint>& _relaxed_constrs) :
		LagrangianMasterProblem(_relaxed_constrs),
		current_subgradients(_relaxed_constrs.size(), 0),
		stepsize(0),
		stepscale(2),
		nstalls(0),
		best_lag_dual(numeric_limits<double>::infinity()),
		ncalls(0)
	{
	}

	// Functions from base class

	void update(const vector<int>& sp_optsol, double sp_optval, const vector<double>& lambdas);

	double solve(vector<double>& lambdas);
};


#endif // LG_MASTER_SUBGRADIENT_HPP_