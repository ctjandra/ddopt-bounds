#ifndef LG_SUBPROB_SUBRESTRICTED_HPP_
#define LG_SUBPROB_SUBRESTRICTED_HPP_

#include "lg_subprob.hpp"

/**
 * Wraps an oracle applying a mapping between variables to the oracle. Assume oracle space is a subspace of original space.
 * Uses given fixed variables for variables that are not in this subspace.
 */

class LagrangianSubproblemOracleSubspaceRestricted : public LagrangianSubproblemOracle
{
private:
	LagrangianSubproblemOracle* oracle;

	// Assume oracle space is a subspace of original space
	vector<int> oracle_to_original_var;   /**< mapping from variables in oracle to LagrangianConstraints */
	vector<int> fixed_vars;               /**< fixed variables; -1 if in subspace, or fixed value otherwise */
	// The mapping takes precedence; fixed variables inside the subspace will be treated as unfixed
	// Note: Currently this is precedence is for an arbitrary reason

public:

	LagrangianSubproblemOracleSubspaceRestricted(LagrangianSubproblemOracle* _oracle, const vector<int>& _oracle_to_original_var,
	        const vector<int>& _fixed_vars) :
		oracle(_oracle), oracle_to_original_var(_oracle_to_original_var), fixed_vars(_fixed_vars) {}

	~LagrangianSubproblemOracleSubspaceRestricted()
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
			// Note: Does not require that fixed_vars[oracle_to_original_var[i]] == DD_UNFIXED_VAR, but will ignore such fixings
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

		// Set variables not in oracle space to the values from fixed_vars
		optsol.clear();
		optsol = fixed_vars; // copy fixed_vars

		// Replace solution from oracle
		for (int i = 0; i < nvars_oracle; ++i) {
			optsol[oracle_to_original_var[i]] = oracle_optsol[i];
		}

		// Recalculate optval with updated optsol (from scratch)
		optval = get_optimal_value(obj, optsol);

		return optval;
	}
};

#endif // LG_SUBPROB_SUBRESTRICTED_HPP_