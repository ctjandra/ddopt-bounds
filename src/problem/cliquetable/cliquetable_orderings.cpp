/*
 * --------------------------------------------------------
 * Ordering class - implementation
 * --------------------------------------------------------
 */

#include <algorithm>

#include "cliquetable_orderings.hpp"
#include "cliquetable_scc.hpp"

using namespace std;


// minimum degree ordering
void MinDegreeCliqueTableOrdering::construct_ordering()
{

	v_in_layer.clear();

	// variable "degree" is the sum of degrees of non-negated and negated nodes

	int nvars = inst->nvars;
	int nvertices = 2 * inst->nvars;

	vector<int> degree(nvertices, 0);
	for (int i = 0; i < nvertices; ++i) {
		for (int j = i+1; j < nvertices; ++j) {
			if (inst->adj[i].contains(j)) {
				assert(inst->adj[j].contains(i));
				++(degree[i]);
				++(degree[j]);
			}
		}
	}

	vector<bool> selected(nvars, false);

	while ((int)v_in_layer.size() < nvars) {

		int min = INF;
		int v = -1;

		for (int i = 0; i < nvars; ++i) {
			if (degree[i] + degree[i+nvars] > 0 && degree[i] + degree[i+nvars] < min && !selected[i]) {
				min = degree[i] + degree[i+nvars];
				v = i;
			}
		}

		if (v == -1) {
			for (int i = 0; i < nvars; ++i) {
				if (!selected[i]) {
					//cout << "\n selected vertex " << i << " --> degree: " << degree[i] + degree[i+nvars] << endl;
					v_in_layer.push_back(i);
				}
			}
		} else {
			selected[v] = true;
			v_in_layer.push_back(v);
			//cout << "\n selected vertex " << v << " --> degree: " << degree[v] + degree[v+nvars] << endl;
			for (int i = 0; i < nvertices; ++i) {
				if (i != v && i != v + nvars) {
					if (inst->adj[i].contains(v)) {
						--(degree[i]);
					}
					if (inst->adj[i].contains(v + nvars)) {
						--(degree[i]);
					}
				}
			}
		}
	}
}


int MinInStateCliqueTableOrdering::select_next_var(int layer)
{
	int min = INF;
	int selected_var = -1;
	for (int i = 0; i < inst->nvars; i++) {
		if (!processed_vars[i] && in_state_counter[i] < min) {
			selected_var = i;
			min = in_state_counter[i];
		}
	}

	processed_vars[selected_var] = true;

	assert(selected_var >= 0);

	return selected_var;
}


void MinInStateCliqueTableOrdering::cb_state_created(State* state)
{
	CliqueTableState* state_ct = dynamic_cast<CliqueTableState*>(state);

	// Increment active state counter if variable is unfixed; i.e. both literals appear in state
	int v = state_ct->intset.get_first();
	while (v != state_ct->intset.get_end() && v < inst->nvars) { // traverse only the nonnegated literals
		// If found, check if its negated counterpart also exists (or if we only look at nonnegated literals)
		if (inst->nonnegated_only || state_ct->intset.contains(inst->get_complement(v))) {
			in_state_counter[v]++;
		}
		v = state_ct->intset.get_next(v);
	}
}


void MinInStateCliqueTableOrdering::cb_state_removed(State* state)
{
	CliqueTableState* state_ct = dynamic_cast<CliqueTableState*>(state);

	// Decrement active state counter if variable is unfixed; i.e. both literals appear in state
	int v = state_ct->intset.get_first();
	while (v != state_ct->intset.get_end() && v < inst->nvars) { // traverse only the nonnegated literals
		// If found, check if its negated counterpart also exists (or if we only look at nonnegated literals)
		if (inst->nonnegated_only || state_ct->intset.contains(inst->get_complement(v))) {
			in_state_counter[v]--;
		}
		v = state_ct->intset.get_next(v);
	}
}
