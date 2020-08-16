#ifndef LG_SUBPROB_FEAS_HPP_
#define LG_SUBPROB_FEAS_HPP_

#include <vector>
#include "lg_subprob.hpp"
#include "../util/options.hpp"
#include "../util/output_stats.hpp"

using namespace std;

class FeasibilityChecker
{
public:
	virtual ~FeasibilityChecker() {}

	/** Checks if candidate is feasible and supplies primal solution to solver */
	virtual bool check_feasibility_and_apply(const vector<int>& candidate, double optval) = 0;
};


/** Oracle that wraps another oracle and checks the feasibility of the optimal solution it outputs to generate primal bounds. */
class LagrangianSubproblemOracleFeasibilityCheck : public LagrangianSubproblemOracle
{
private:
	LagrangianSubproblemOracle* oracle;       /**< oracle that this wrapper calls */

	FeasibilityChecker* feas_checker;         /**< function to check feasibility */
	const vector<double>& true_obj;           /**< problem objective */
	const vector<int>& fixed_vars;            /**< fixed variables; -1 means unfixed */

	vector<double> primal_sol;                /**< best primal solution found by oracle; empty if none found */
	double primal_bound;                      /**< current primal bound (can be initialized) */

	Options *options;
	OutputStats *output_stats;

public:

	LagrangianSubproblemOracleFeasibilityCheck(LagrangianSubproblemOracle* _oracle, FeasibilityChecker* _feas_checker,
	        const vector<double>& _true_obj, const vector<int>& _fixed_vars, double _primal_bound, Options* _options,
	        OutputStats* _output_stats) :
		oracle(_oracle), feas_checker(_feas_checker), true_obj(_true_obj), fixed_vars(_fixed_vars),
		primal_bound(_primal_bound), options(_options), output_stats(_output_stats) {}

	~LagrangianSubproblemOracleFeasibilityCheck()
	{
		delete oracle;
		delete feas_checker;
	}

	double solve(const vector<double>& obj, vector<int>& optsol);
};


#endif // LG_SUBPROB_FEAS_HPP_