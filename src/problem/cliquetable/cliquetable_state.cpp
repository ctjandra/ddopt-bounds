#include "cliquetable_state.hpp"
#include <boost/dynamic_bitset.hpp>

State* CliqueTableState::transition(Problem* prob, int var, int val)
{
	assert(val == 0 || val == 1);

	if (val == 1 && !intset.contains(var)) {
		return NULL;
	}

	CliqueTableInstance* insti = dynamic_cast<CliqueTableInstance*>(prob->inst);
	if (insti == NULL) {
		cout << "Error: Using incompatible State and Instance" << endl;
		exit(1);
	}

	int nvars = insti->nvars;

	if (!insti->nonnegated_only) {
		if (val == 0 && !intset.contains(var + nvars)) {
			return NULL;
		}
	}

	CliqueTableState* new_state;
	new_state = new CliqueTableState(intset);

	if (insti->nonnegated_only) {

		// Remove vertex itself
		new_state->intset.remove(var);

		// Remove neighbors of vertex if added to graph
		if (val == 1) {
			new_state->intset.set &= insti->adj_mask_compl[var].set;
		}

	} else {

		// Remove variable itself (both non-negated and negated)
		new_state->intset.remove(var);
		new_state->intset.remove(var + nvars);

		// Remove neighbors of vertex if added to graph
		if (val == 1) {
			new_state->intset.set &= insti->adj_mask_compl[var].set;
		} else { // val == 0
			new_state->intset.set &= insti->adj_mask_compl[var+nvars].set;
		}

		// Check for early infeasibility: if any of the vertices removed from set has a counterpart also not in set, then infeasible
		// Note that we cannot simply check if neither of them are in the set because they may have been already branched upon

		// 1 if a set-to-zero variable (1 in first expression) has a corresponding negation set to zero (0 in second expression)
		boost::dynamic_bitset<>& new_set = new_state->intset.set;
		boost::dynamic_bitset<> infeas_check = (intset.set - new_set) - ((new_set << nvars) | (new_set >> nvars));
		infeas_check.reset(var); // ignore current var
		infeas_check.reset(var + nvars);
		if (infeas_check.any()) { // if any 1 is left (except for current variable), then infeasible
			// infeasible
			delete new_state;
			return NULL;
		}
	}

	return new_state;
}
