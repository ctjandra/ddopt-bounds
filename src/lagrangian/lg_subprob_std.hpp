#ifndef LG_SUBPROB_STD_HPP_
#define LG_SUBPROB_STD_HPP_

#include <vector>
#include <iostream>
#include "../util/util.hpp"
#include "lg_subprob.hpp"
#include "lg_constraint.hpp"

using namespace std;

/** Standard implementation of a Lagrangian subproblem with an oracle */
class LagrangianSubproblemStandard : public LagrangianSubproblem
{
private:
	int nvars;                                     /**< number of variables */
	vector<double> obj;                            /**< objective function of the original problem */
	vector<LagrangianConstraint> relaxed_constrs;  /**< constraints relaxed in Lagrangian relaxation */
	LagrangianSubproblemOracle* oracle;            /**< oracle to call */

public:

	LagrangianSubproblemStandard(int _nvars, const vector<double>& _obj, const vector<LagrangianConstraint>& _relaxed_constrs,
	                             LagrangianSubproblemOracle* _oracle) :
		nvars(_nvars), obj(_obj), relaxed_constrs(_relaxed_constrs), oracle(_oracle)
	{
		assert(oracle != NULL);
		assert((int) obj.size() == nvars);
	}

	double solve(const vector<double>& lambdas, vector<int>& optsol);

};

#endif // LG_SUBPROB_STD_HPP_
