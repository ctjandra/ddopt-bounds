#include <cstdlib>
#include <iostream>
#include <vector>

#include "cliquetable_instance.hpp"
#include "cliquetable_cons_id.hpp"
#include "../bp/cons_id.hpp"

using namespace std;


CliqueTableInstance::CliqueTableInstance(SCIP* scip, bool include_ct_rows, bool mask_transitive)
{
	// Assume SCIP is mid-solve
	assert(SCIPgetStage(scip) == SCIP_STAGE_SOLVING);

	SCIP_COL** cols;
	SCIP_CALL_ABORT(SCIPgetLPColsData(scip, &cols, &nvars));

	vector<int> scipvar_to_ctvar(nvars);
	for (int i = 0; i < nvars; ++nvars) {
		scipvar_to_ctvar[i] = i; // Identity map
	}

	init_adj(cols, scipvar_to_ctvar);
	convert_clique_table(cols, scipvar_to_ctvar);
	if (include_ct_rows) {
		convert_clique_table_rows(scip, scipvar_to_ctvar);
	}
	update_nonnegated_only();

	if (mask_transitive) {
		create_complement_mask_with_transitivities();
	} else {
		create_complement_mask();
	}
	extract_weights(scip, cols, scipvar_to_ctvar);
}


CliqueTableInstance::CliqueTableInstance(SCIP* scip, SCIP_COL** cols, int ncols, const vector<int>& scipvar_to_ctvar,
        bool include_ct_rows, bool mask_transitive)
{
	nvars = ncols;

	init_adj(cols, scipvar_to_ctvar);
	convert_clique_table(cols, scipvar_to_ctvar);
	if (include_ct_rows) {
		convert_clique_table_rows(scip, scipvar_to_ctvar);
	}
	update_nonnegated_only();

	if (mask_transitive) {
		create_complement_mask_with_transitivities();
	} else {
		create_complement_mask();
	}
	extract_weights(scip, cols, scipvar_to_ctvar);
}


void CliqueTableInstance::init_adj(SCIP_COL** cols, const vector<int>& scipvar_to_ctvar)
{
	adj.resize(2 * nvars);

	for (int c = 0; c < 2 * nvars; ++c) {
		adj[c].resize(0, (2 * nvars)-1, false);
	}

	// Make the negated variable adjacent to the positive one and vice versa
	for (int c = 0; c < nvars; ++c) {
		SCIP_VAR* var = SCIPcolGetVar(cols[c]);
		int var_idx = scipvar_to_ctvar[SCIPvarGetProbindex(var)];
		assert(var_idx < nvars); // index should be in [0..nvars-1]

		adj[var_idx].add(var_idx + nvars);
		adj[var_idx+nvars].add(var_idx);
	}
}


void CliqueTableInstance::update_nonnegated_only()
{
	// Mark if negated nodes are only adjacent to corresponding non-negated ones
	nonnegated_only = true;
	int size = adj.size();
	for (int v = nvars; v < size; v++) {
		if (adj[v].get_size() > 1) {
			nonnegated_only = false;
			break;
		}
	}
}


/**
 * Convert the clique table into an intset vector of adjacency. Indices n+i correspond to negated versions of variable i.
 * Uses a map of variables from the original space to the one given by cols; variables not in space must be negative in map.
 */
void CliqueTableInstance::convert_clique_table(SCIP_COL** cols, const vector<int>& scipvar_to_ctvar)
{
	for (int c = 0; c < nvars; ++c) {
		SCIP_VAR* var = SCIPcolGetVar(cols[c]);
		int var_idx = scipvar_to_ctvar[SCIPvarGetProbindex(var)];
		assert(var_idx >= 0 && var_idx < nvars); // index should be in [0..nvars-1]

		for (SCIP_Bool val = FALSE; val <= TRUE; ++val) {
			int idx = (val == TRUE) ? var_idx : var_idx + nvars;

			int ncliques = SCIPvarGetNCliques(var, val);
			SCIP_CLIQUE** cliques = SCIPvarGetCliques(var, val);

			for (int k = 0; k < ncliques; ++k) {
				SCIP_VAR** clique_vars = SCIPcliqueGetVars(cliques[k]);
				SCIP_Bool* clique_vals = SCIPcliqueGetValues(cliques[k]);
				int clique_nvars = SCIPcliqueGetNVars(cliques[k]);

				for (int u = 0; u < clique_nvars; ++u) {
					SCIP_VAR* cvar = clique_vars[u];
					int cidx = scipvar_to_ctvar[SCIPvarGetProbindex(cvar)];
					if (cidx < 0) { // This variable is not in subspace
						continue;
					}
					assert(cidx < nvars); // index should be in [0..nvars-1]
					if (clique_vals[u] == FALSE) {
						cidx += nvars; // negated variable
					}

					if (idx == cidx) {
						continue; // skip own variable
					}

					// Add adjacency
					adj[idx].add(cidx);
					adj[cidx].add(idx);
				}
			}
		}
	}

	int size = adj.size();
	for (int v = 0; v < size; v++) {
		assert(!adj[v].contains(v)); // no self-loops; mask takes care of removing own variable
		assert(adj[v].contains((v < nvars) ? v + nvars : v - nvars));
	}
}


void CliqueTableInstance::convert_clique_table_rows(SCIP* scip, const vector<int>& scipvar_to_ctvar)
{
	SCIP_ROW** rows;
	int nrows;

	SCIP_CALL_ABORT(SCIPgetLPRowsData(scip, &rows, &nrows));

	for (int i = 0; i < nrows; ++i) {
		SCIP_ROW* row = rows[i];
		bool lhs_ctform;
		bool rhs_ctform;
		is_row_clique_table_form(scip, row, &lhs_ctform, &rhs_ctform);
		if (lhs_ctform || rhs_ctform) {
			int nnonz = SCIProwGetNNonz(row);
			SCIP_COL** row_cols = SCIProwGetCols(row);
			SCIP_Real* row_vals = SCIProwGetVals(row);
			for (int j = 0; j < nnonz; ++j) {
				for (int k = j + 1; k < nnonz; ++k) {
					int vj = scipvar_to_ctvar[SCIPvarGetProbindex(SCIPcolGetVar(row_cols[j]))];
					int vk = scipvar_to_ctvar[SCIPvarGetProbindex(SCIPcolGetVar(row_cols[k]))];
					if (vj >= 0 && vk >= 0) {
						assert(vj < nvars);
						assert(vk < nvars);
						// if <= constraint, the negative variables are the negated ones
						// if >= constraint, the positive variables are the negated ones
						if ((rhs_ctform && row_vals[j] < 0) || (lhs_ctform && row_vals[j] > 0)) {
							vj += nvars; // negate vj
						}
						if ((rhs_ctform && row_vals[k] < 0) || (lhs_ctform && row_vals[k] > 0)) {
							vk += nvars; // negate vk
						}
						adj[vj].add(vk);
						adj[vk].add(vj);
					}
				}
			}
		}
	}
}


void CliqueTableInstance::create_complement_mask()
{
	int size = adj.size();
	adj_mask_compl.resize(size);
	assert(size == 2 * nvars);
	for (int v = 0; v < size; v++) {
		adj_mask_compl[v].resize(0, size-1, false);
		adj_mask_compl[v].set = ~(adj[v].set);
		adj_mask_compl[v].remove(v);
	}

	if (nonnegated_only) {
		for (int v = 0; v < size; v++) {
			adj_mask_compl[v].resize(0, nvars-1);
		}
	}
}


void CliqueTableInstance::create_complement_mask_with_transitivities()
{
	int size = adj.size();
	adj_mask_compl.resize(size);
	assert(size == 2 * nvars);

	// Complement of edges
	for (int v = 0; v < size; v++) {
		adj_mask_compl[v].resize(0, size-1, false);
		adj_mask_compl[v].set = ~(adj[v].set);

		int j;
		for (j = 0; j < size; ++j) {
			boost::dynamic_bitset<> new_set = adj_mask_compl[v].set;
			// Iterate through variables with domains of size one
			for (int u = adj_mask_compl[v].get_first(); u != adj_mask_compl[v].get_end(); u = adj_mask_compl[v].get_next(u)) {
				if (!new_set.test(get_complement(u))) {
					new_set &= ~(adj[u].set);
				}
			}
			if (new_set == adj_mask_compl[v].set) {
				break;
			}
			adj_mask_compl[v].set = new_set;
		}
		assert(j < size); // there should be no more than size - 1 iterations

		adj_mask_compl[v].remove(v);
	}

	if (nonnegated_only) {
		for (int v = 0; v < size; v++) {
			adj_mask_compl[v].resize(0, nvars-1);
		}
	}
}


void CliqueTableInstance::extract_weights(SCIP* scip, SCIP_COL** cols, const vector<int>& scipvar_to_ctvar)
{
	weights = new double[nvars];
	for (int i = 0; i < nvars; ++i) {
		int idx = scipvar_to_ctvar[SCIPvarGetProbindex(SCIPcolGetVar(cols[i]))];
		assert(idx < nvars);
		weights[idx] = SCIPcolGetObj(cols[i]);
		weights[idx] = -weights[idx]; // switch signs because SCIP minimizes and we maximize
		// Note: To obtain original objective value, we will need to apply SCIPretransformObj(scip, obj) to the final value
	}
}


void CliqueTableInstance::print()
{
	int size = adj.size();
	for (int i = 0; i < size; ++i) {
		cout << i << ": " << adj[i] << endl;
	}
}


void CliqueTableInstance::print_mapped(const vector<int>& subvar_to_var)
{
	int size = adj.size();
	int nsub = subvar_to_var.size();
	for (int i = 0; i < size; ++i) {
		if (i < nsub) {
			cout << subvar_to_var[i] << ": ";
		} else {
			cout << "~" << subvar_to_var[i-nsub] << ": ";
		}
		cout << "[ ";
		int val = adj[i].get_first();
		while (val != adj[i].get_end()) {
			if (val < nsub) {
				cout << subvar_to_var[val] << " ";
			} else {
				cout << "~" << subvar_to_var[val-nsub] << " ";
			}
			val = adj[i].get_next(val);
		}
		cout << "]";
		cout << endl;
	}
}


int CliqueTableInstance::get_number_of_edges()
{
	int nedges = 0;
	int size = adj.size();
	for (int i = 0; i < size; ++i) {
		nedges += adj[i].get_size();
	}
	assert(nedges % 2 == 0);
	return nedges / 2;
}


int CliqueTableInstance::get_complement(int i)
{
	assert(i >= 0 && i < 2 * nvars);
	if (i < nvars) {
		return i + nvars;
	} else {
		return i - nvars;
	}
}
