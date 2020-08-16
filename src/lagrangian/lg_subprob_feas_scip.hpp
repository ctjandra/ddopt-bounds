#ifndef LG_SUBPROB_FEAS_SCIP_HPP_
#define LG_SUBPROB_FEAS_SCIP_HPP_

#include "lg_subprob_feas.hpp"


class FeasibilityCheckerSCIP : public FeasibilityChecker
{
	SCIP* scip;

public:

	FeasibilityCheckerSCIP(SCIP* _scip) : scip(_scip) {}

	bool check_feasibility_and_apply(const vector<int>& candidate, double optval)
	{
		SCIP_Bool feasible;

		SCIP_COL** cols;
		int ncols;
		SCIP_CALL(SCIPgetLPColsData(scip, &cols, &ncols));

		SCIP_VAR** vars;
		SCIP_Real* vals;
		SCIP_CALL(SCIPallocBufferArray(scip, &vars, ncols));
		SCIP_CALL(SCIPallocBufferArray(scip, &vals, ncols));
		for (int j = 0; j < ncols; ++j) {
			SCIP_VAR* scip_var = SCIPcolGetVar(cols[j]);
			int idx = SCIPvarGetProbindex(scip_var);
			assert(idx >= 0 && idx < ncols);
			vars[idx] = scip_var;
			vals[idx] = candidate[idx];
		}

		SCIP_SOL* sol;
		SCIP_CALL(SCIPcreateSol(scip, &sol, NULL));
		SCIP_CALL(SCIPsetSolVals(scip, sol, ncols, vars, vals));
		// SCIP_CALL( SCIPtrySolFree(scip, &sol, TRUE, TRUE, TRUE, TRUE, TRUE, &feasible) ); // print reason
		SCIP_CALL(SCIPtrySolFree(scip, &sol, FALSE, TRUE, TRUE, TRUE, TRUE, &feasible));

		assert(!feasible || DBL_EQ(SCIPgetPrimalbound(scip), optval));

		SCIPfreeBufferArray(scip, &vars);
		SCIPfreeBufferArray(scip, &vals);

		return (feasible == TRUE);
	}
};


#endif // LG_SUBPROB_FEAS_SCIP_HPP_