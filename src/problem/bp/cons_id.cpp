#include "cons_id.hpp"


bool is_set_packing(BPRow* row)
{
	// Canonically structured only (coefficients 1 and <= 1)
	if (row->sense == SENSE_GE) {
		return false;
	}
	if (!DBL_EQ(row->rhs, 1)) {
		return false;
	}
	for (double coeff : row->coeffs) {
		if (!DBL_EQ(coeff, 1)) {
			return false;
		}
	}
	return true;
}

#ifdef SOLVER_SCIP
bool is_set_packing(SCIP_ROW* row)
{
	// Canonically structured only (coefficients 1 and <= 1); ignore left-hand side
	SCIP_Real rhs = SCIProwGetRhs(row);
	if (!DBL_EQ(rhs, 1)) {
		return false;
	}
	SCIP_Real* vals = SCIProwGetVals(row);
	int nnonz = SCIProwGetNNonz(row);
	for (int i = 0; i < nnonz; ++i) {
		if (!DBL_EQ(vals[i], 1)) {
			return false;
		}
	}
	return true;
}
#endif

Graph* extract_set_packing_graph(const vector<BPRow*>& rows, int nvars, vector<int>& var_to_node, vector<int>& node_to_var,
                                 vector<bool>& used_rows)
{
	// Create mapping first
	var_to_node.resize(nvars, -1);
	int nrows = rows.size();
	used_rows.resize(nrows);
	for (int i = 0; i < nrows; ++i) {
		BPRow* row = rows[i];
		if (is_set_packing(row)) {
			for (int j : row->ind) {
				assert(j < nvars);
				var_to_node[j] = 0; // mark first; actual mapping is done later
			}
			used_rows[i] = true;
		} else {
			used_rows[i] = false;
		}
	}

	int node = 0;
	node_to_var.clear();
	for (int i = 0; i < nvars; ++i) {
		if (var_to_node[i] >= 0) {
			var_to_node[i] = node;
			node_to_var.push_back(i);
			node++;
		}
	}

	// Create graph
	int nvertices = node_to_var.size();
	Graph* graph = new Graph(nvertices);
	for (BPRow* row : rows) {
		if (is_set_packing(row)) {
			for (int i = 0; i < row->nnonz; ++i) {
				for (int j = i + 1; j < row->nnonz; ++j) {
					int vi = var_to_node[row->ind[i]];
					int vj = var_to_node[row->ind[j]];
					assert(vi >= 0 && vi < nvertices);
					assert(vj >= 0 && vj < nvertices);
					graph->add_edge(vi, vj);
				}
			}
		}
	}

	return graph;
}
