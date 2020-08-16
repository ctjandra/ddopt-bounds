#ifndef LG_SUBPROB_BDD_HPP_
#define LG_SUBPROB_BDD_HPP_

#include <vector>
#include "lg_subprob.hpp"
#include "../bdd/bdd.hpp"

/** Simple oracle that returns the optimal solution in a BDD. */
class LagrangianSubproblemOracleBDD : public LagrangianSubproblemOracle
{
private:
	BDD* bdd;

public:

	LagrangianSubproblemOracleBDD(BDD* _bdd) : bdd(_bdd) {}

	/** Calculate optimal solution in a BDD. */
	double solve(const vector<double>& obj, vector<int>& optsol)
	{
		assert(bdd != NULL);
		double optval = bdd->get_optimal_sol(obj, optsol, true);
		return optval;
	}
};


#endif // LG_SUBPROB_BDD_HPP_