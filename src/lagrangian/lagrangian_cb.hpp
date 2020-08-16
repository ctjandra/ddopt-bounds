#ifndef LAGRANGIAN_CB_HPP_
#define LAGRANGIAN_CB_HPP_

#ifdef USE_CONICBUNDLE

#include "CBSolver.hxx"
using namespace ConicBundle;

#endif // USE_CONICBUNDLE

#include <vector>
#include "lagrangian.hpp"
#include "lg_subprob.hpp"
#include "lg_constraint.hpp"
#include "../util/stats.hpp"
#include "../util/options.hpp"

using namespace std;


/**
 * Compute Lagrangian relaxation via the ConicBundle library.
 * Implementation structure differs from LagrangianRelaxation class because ConicBundle takes over the master.
 */
class LagrangianRelaxationCB
{
private:
	vector<LagrangianConstraint> relaxed_constrs;  /**< constraints Lagrangian relaxation is applied to */
	LagrangianSubproblem*        subproblem;       /**< subproblem for the Lagrangian relaxation */
	double                       time_limit;       /**< time limit for the Lagrangian relaxation */
	Options*                     options;

public:

	LagrangianRelaxationCB(const vector<LagrangianConstraint>& _relaxed_constrs,
	                       LagrangianSubproblem* _subproblem, double _time_limit,
	                       Options* _options) : relaxed_constrs(_relaxed_constrs), subproblem(_subproblem),
		time_limit(_time_limit), options(_options)
	{
#ifndef USE_CONICBUNDLE
		cout << "Error: ConicBundle not compiled" << endl;
		exit(1);
#endif // USE_CONICBUNDLE
	}

	~LagrangianRelaxationCB() {}

	double solve(LagrangianRelaxationParams params);
};

#ifdef USE_CONICBUNDLE

/** Subproblem wrapper for ConicBundle */
class LagrangianSubproblemCB : public FunctionOracle
{
private:
	vector<LagrangianConstraint> relaxed_constrs;
	LagrangianSubproblem* subprob;
	Stats stats;
	int neval;

public:

	LagrangianSubproblemCB(const vector<LagrangianConstraint>& _relaxed_constrs, LagrangianSubproblem* _subprob) :
		relaxed_constrs(_relaxed_constrs), subprob(_subprob)
	{
		stats.register_name("subprob");
		neval = 0;
	}

	int evaluate(const DVector& lambdas, double relprec, double& objval, DVector& cut_vals,
	             vector<DVector>& subgradients, vector<PrimalData*>& primal_solutions, PrimalExtender*&)
	{
		stats.start_timer(0);

		// Compute subproblem
		vector<int> sp_optsol;
		objval = subprob->solve(lambdas, sp_optsol);

		// Return new subproblem value
		cut_vals.push_back(objval);

		// Return new subgradient
		int nrows_lag = relaxed_constrs.size();
		DVector subg(nrows_lag);
		for (int i = 0; i < nrows_lag; ++i) {
			subg[i] = relaxed_constrs[i].get_lagrangian_subgradient(sp_optsol);
		}
		subgradients.push_back(subg);

		stats.end_timer(0);
		neval++;

		return 0;
	}

	double get_time()
	{
		return stats.get_time(0);
	}

	int get_number_evaluations()
	{
		return neval;
	}
};

#endif // USE_CONICBUNDLE

#endif // LAGRANGIAN_CB_HPP_