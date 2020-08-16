#ifndef CONS_ID_HPP_
#define CONS_ID_HPP_

/**
 * Helper functions to identify types of rows
 */

#include "bprow.hpp"
#include "../../util/util.hpp"
#include "../../util/graph.hpp"

#ifdef SOLVER_SCIP
#include "scip/scip.h"
#endif

/** Return true if the given row is a properly structured set packing constraint (with <=, rhs and coefficients 1) */
bool is_set_packing(BPRow* row);

/**
 * Create a graph from set packing rows; stores in var_to_node and node_to_var mappings between BPRow variables and nodes, and
 * in used_rows the rows that were used for the graph (in the same order as rows).
 */
Graph* extract_set_packing_graph(const vector<BPRow*>& rows, int nvars, vector<int>& var_to_node, vector<int>& node_to_var,
                                 vector<bool>& used_rows);


#ifdef SOLVER_SCIP

/** Return true if the rhs of a given row is a properly structured set packing constraint (ignores lhs) */
bool is_set_packing(SCIP_ROW* row);

#endif

#endif // CONS_ID_HPP_
