#include "cliquetable_problem.hpp"

/* Callbacks */

bool CliqueTableProblem::cb_skip_var_for_long_arc(int var, State* state)
{
	CliqueTableState* state_is = dynamic_cast<CliqueTableState*>(state);

	if (instance->nonnegated_only) {
		// Easier case (independent set)
		return !state_is->intset.contains(var);
	}

	// If negated nodes exist, setting a variable to zero may have an effect; we only create long arcs if it does not

	if (state_is->intset.contains(var)) {
		return false;
	}

	// Redundant, but in many cases the negated node is only adjacent to the non-negated node and we do not need to check
	// if the transition has no effect
	if (instance->adj[var+instance->nvars].get_size() == 1) {
		state_is->mark_as_processed(var);
		return true;
	}

	// Return true if setting variable to zero has no effect
	if ((state_is->intset.set & instance->adj_mask_compl[var+instance->nvars].set) == state_is->intset.set) {
		state_is->mark_as_processed(var);
		return true;
	}

	return false;
}


void CliqueTableProblem::cb_layer_end(int current_var)
{
	// Update minactivity and maxactivity for propagator
	if (prop != NULL) {
		prop->update_layer_end(current_var);
	}
}
