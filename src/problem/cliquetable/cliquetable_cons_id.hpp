#ifndef CLIQUETABLE_CONS_ID_HPP_
#define CLIQUETABLE_CONS_ID_HPP_

#include "scip/scip.h"
#include <iostream>


/**
 * Return true if constraint is of form:
 *   sum_P x_i - sum_N x_j <= 1 - |N|, or
 *   sum_P x_i - sum_N x_j >= |P| - 1
 * in ret_rhs and ret_lhs respectively. Scaling is taken into account, but not dominance.
 */
void is_row_clique_table_form(SCIP* scip, SCIP_ROW* row, bool* ret_lhs, bool* ret_rhs);

/**
 * Return true if a row being in clique table form implies it is in the clique table.
 * Should typically return true if clique table construction captures sufficient information, but may return false.
 */
bool check_clique_table_form_consistent(SCIP* scip, SCIP_ROW* row);

/** Return true if signed variables are adjacent in the clique table */
bool is_adjacent_in_clique_table(SCIP_VAR* var_i, SCIP_VAR* var_j, SCIP_Bool sign_i, SCIP_Bool sign_j);

#endif // CLIQUETABLE_CONS_ID_HPP_