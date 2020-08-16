#ifndef LG_SUBPROB_SUBRELAXED_HPP_
#define LG_SUBPROB_SUBRELAXED_HPP_

#include "lg_subprob.hpp"

/**
 * Wraps an oracle applying a mapping between variables to the oracle. Assume oracle space is a subspace of original space.
 * Constrains the oracle into a 0-1 cube for variables that are not in this subspace.
 */
class LagrangianSubproblemOracleSubspaceRelaxed : public LagrangianSubproblemOracle
{
private:
	LagrangianSubproblemOracle* oracle;

	// Assume oracle space is a subspace of original space
	vector<int> oracle_to_original_var;   /**< mapping from variables in oracle to LagrangianConstraints */

public:

	LagrangianSubproblemOracleSubspaceRelaxed(LagrangianSubproblemOracle* _oracle, const vector<int>& _oracle_to_original_var) :
		oracle(_oracle), oracle_to_original_var(_oracle_to_original_var) {}

	~LagrangianSubproblemOracleSubspaceRelaxed()
	{
		delete oracle;
	}

	double solve(const vector<double>& obj, vector<int>& optsol)
	{
		int nvars_original = obj.size();
		int nvars_oracle = oracle_to_original_var.size();

		assert(nvars_oracle <= nvars_original); // assume oracle space is a subspace of original space

		// Map original to objective to oracle space (reduce to a possibly smaller space)
		vector<double> oracle_obj(nvars_oracle);
		for (int i = 0; i < nvars_oracle; ++i) {
			oracle_obj[i] = obj[oracle_to_original_var[i]];
		}

		// // Debugging info
		// cout << "Lagrangian oracle objective: ";
		// for (double k : oracle_obj) {
		// 	cout << k << " ";
		// }
		// cout << endl;

		// Solve oracle
		vector<int> oracle_optsol;
		double optval = oracle->solve(oracle_obj, oracle_optsol);

		// If no solution is found, do nothing
		if (oracle_optsol.size() == 0) {
			return optval;
		}

		// Set variables not in oracle space to zero if obj is negative; one if obj is positive
		optsol.clear();
		optsol.resize(nvars_original, 0);

		// All variables are set according to objective for now; these will be replaced by the BDD solution next
		for (int i = 0; i < nvars_original; ++i) {
			if (DBL_GT(obj[i], 0)) {
				optsol[i] = 1;
			} else {
				optsol[i] = 0;
			}
		}
		for (int i = 0; i < nvars_oracle; ++i) {
			optsol[oracle_to_original_var[i]] = oracle_optsol[i];
		}

		// Recalculate optval with optsol updated with points from 0-1 cube (from scratch)
		optval = get_optimal_value(obj, optsol);

		return optval;
	}
};

#endif // LG_SUBPROB_SUBRELAXED_HPP_