#ifndef CLIQUETABLE_COMPLETION_HPP_
#define CLIQUETABLE_COMPLETION_HPP_

#include "../../core/completion.hpp"
#include "cliquetable_state.hpp"


/** Use domains in state as the completion bounds. */
class CliqueTableDomainCompletionBound : public CompletionBound
{
	double dual_bound(Instance* inst, Node* node, Node* parent)
	{
		CliqueTableState* state = dynamic_cast<CliqueTableState*>(node->state);
		CliqueTableInstance* inst_ct = dynamic_cast<CliqueTableInstance*>(inst);

		// Calculate the largest objective value that the variables can take satisfying domains; that is,
		// sum all positive objective coefficients for variables that can take value one
		//   + all negative objective coefficients for variables that must take value one
		// Note: Processed variables are not taken into account because they are not in the set (in either case)
		// This assumes we are maximizing in the decision diagram (we should be).
		double bound = 0;
		int nvars = inst_ct->nvars;
		if (inst_ct->nonnegated_only) {
			// Domains are either zero or zero-one
			for (int i = state->intset.get_first(); i != state->intset.get_end(); i = state->intset.get_next(i)) {
				assert(i >= 0 && i < nvars);
				// Variable i can take value one
				if (DBL_GT(inst_ct->weights[i], 0)) {
					bound += inst_ct->weights[i];
				}
			}
		} else {
			for (int i = state->intset.get_first(); i != state->intset.get_end(); i = state->intset.get_next(i)) {
				assert(i >= 0 && i < 2 * nvars);
				if (i < nvars) {
					// Variable i can take value one
					if (DBL_GT(inst_ct->weights[i], 0)) {
						bound += inst_ct->weights[i];
					} else if (DBL_LT(inst_ct->weights[i], 0) && !state->intset.contains(i + nvars)) {
						// Domain contains value one but not zero; add (negative) objective coefficient
						bound += inst_ct->weights[i];
					}
				}
			}
		}

		return bound;
	}

	double primal_bound(Instance* inst, Node* node, Node* parent)
	{
		CliqueTableState* state = dynamic_cast<CliqueTableState*>(node->state);
		CliqueTableInstance* inst_ct = dynamic_cast<CliqueTableInstance*>(inst);

		// Calculate the smallest objective value that the variables can take satisfying domains; that is,
		// sum all negative objective coefficients for variables that can take value one
		//   + all positive objective coefficients for variables that must take value one
		// Note: Processed variables are not taken into account because they are not in the set (in either case)
		// This assumes we are maximizing in the decision diagram (we should be).
		double bound = 0;
		int nvars = inst_ct->nvars;
		if (inst_ct->nonnegated_only) {
			// Domains are either zero or zero-one
			for (int i = state->intset.get_first(); i != state->intset.get_end(); i = state->intset.get_next(i)) {
				assert(i >= 0 && i < nvars);
				// Variable i can take value one
				if (DBL_LT(inst_ct->weights[i], 0)) {
					bound += inst_ct->weights[i];
				}
			}
		} else {
			for (int i = state->intset.get_first(); i != state->intset.get_end(); i = state->intset.get_next(i)) {
				assert(i >= 0 && i < 2 * nvars);
				if (i < nvars) {
					// Variable i can take value one
					if (DBL_LT(inst_ct->weights[i], 0)) {
						bound += inst_ct->weights[i];
					} else if (DBL_GT(inst_ct->weights[i], 0) && !state->intset.contains(i + nvars)) {
						// Domain contains value one but not zero; add (positive) objective coefficient
						bound += inst_ct->weights[i];
					}
				}
			}
		}

		return bound;
	}
};

#endif /* CLIQUETABLE_COMPLETION_HPP_ */
