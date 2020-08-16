#ifndef LG_SUBPROB_NRP_HPP_
#define LG_SUBPROB_NRP_HPP_

#include <vector>
#include "lg_subprob.hpp"
#include "../bdd/bdd.hpp"

// Note: This oracle is not meant to be used with the Lagrangian framework;
// it is created for primal bound generation and it happens to fit well with the existing structure

/** Simple oracle that returns the optimal non-relaxed solution in a BDD. */
class LagrangianSubproblemOracleNRP : public LagrangianSubproblemOracle
{
private:
	BDD* bdd;

public:

	LagrangianSubproblemOracleNRP(BDD* _bdd) : bdd(_bdd) {}

	/** Calculate optimal solution in a BDD. */
	double solve(const vector<double>& obj, vector<int>& optsol)
	{
		assert(bdd != NULL);
		double optval = bdd->get_optimal_sol(obj, optsol, true, true);
		return optval;
	}
};


#endif // LG_SUBPROB_NRP_HPP_