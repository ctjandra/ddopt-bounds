#include "lg_dd_selector_ct_scip.hpp"

#include "../problem/bp/prop_linearcons.hpp"
#include "../problem/bp/bp_reader_scip.hpp"
#include "../problem/cliquetable/cliquetable_cons_id.hpp"
#include "../problem/cliquetable/cliquetable_instance.hpp"
#include "../problem/cliquetable/cliquetable_problem.hpp"
#include "../problem/cliquetable/cliquetable_orderings.hpp"
#include "../problem/cliquetable/cliquetable_scc.hpp"
#include "../problem/cliquetable/ct_prop_linearcons.hpp"
#include "../core/mergers.hpp"


bool LagrangianDDConstraintSelectorCliqueTable::exists_structure(SCIP* scip, SCIP_ROW** rows, int nrows)
{
	// Check if clique table is not empty
	return (SCIPgetNCliques(scip) > 0);
}

SCIPRowVector* LagrangianDDConstraintSelectorCliqueTable::extract_lagrangian_rows(SCIP* scip, SCIP_ROW** rows,
        int nrows, Options* options)
{
	SCIPRowVector* lagrangian_rows = new SCIPRowVector();

	for (int i = 0; i < nrows; ++i) {
		bool clq_lhs;
		bool clq_rhs;

		// Store in clq_lhs and clq_rhs if LHS or RHS are of clique table form
		is_row_clique_table_form(scip, rows[i], &clq_lhs, &clq_rhs);

		if (!options->lag_add_all_ct_rows && !check_clique_table_form_consistent(scip, rows[i])) {
			cout << "Warning: Row " << i << " of clique table form is not in clique table; added row to Lagrangian" << endl;
			SCIPprintRow(scip, rows[i], NULL);
			clq_lhs = false;
			clq_rhs = false;
		}

		SCIP_Real lhs = SCIProwGetLhs(rows[i]);
		SCIP_Real rhs = SCIProwGetRhs(rows[i]);

		// Add row to Lagrangianize if not a clique table row and not unused (infinite) LHS/RHS
		lagrangian_rows->push_back(rows[i], !clq_lhs && !SCIPisInfinity(scip, -lhs), !clq_rhs && !SCIPisInfinity(scip, rhs));
	}

	// // Debugging info: SCIP clique table
	// {
	// 	cout << "Clique table:" << endl;
	// 	SCIP_CLIQUE** cliques = SCIPgetCliques(scip);
	// 	int ncliques = SCIPgetNCliques(scip);
	// 	for (int k = 0; k < ncliques; ++k) {
	// 		SCIP_VAR** clique_vars = SCIPcliqueGetVars(cliques[k]);
	// 		SCIP_Bool* clique_vals = SCIPcliqueGetValues(cliques[k]);
	// 		int nvars = SCIPcliqueGetNVars(cliques[k]);
	// 		for (int u = 0; u < nvars; ++u)	{
	// 			if (clique_vals[u] == TRUE) {
	// 				cout << "+";
	// 			} else {
	// 				cout << "-";
	// 			}
	// 			cout << SCIPvarGetName(clique_vars[u]) << " ";
	// 		}
	// 		cout << endl;
	// 	}
	// }
	// cout << "Lagrangian rows size:  " << lagrangian_rows->rows.size() << endl;

	return lagrangian_rows;
}

void LagrangianDDConstraintSelectorCliqueTable::prepare_dd_construction(SCIP* scip, SCIP_ROW** rows, int nrows,
        Options* options)
{
	// Extract set packing rows
	vector<bool> prop_skip_rows_lhs(nrows, false); // For propagator in decision diagram
	vector<bool> prop_skip_rows_rhs(nrows, false); // For propagator in decision diagram
	for (int i = 0; i < nrows; ++i) {
		bool clq_lhs;
		bool clq_rhs;
		is_row_clique_table_form(scip, rows[i], &clq_lhs, &clq_rhs);
		prop_skip_rows_lhs[i] = clq_lhs;
		prop_skip_rows_rhs[i] = clq_rhs;
	}

	// Create rows that will be considered in propagation in the DD; these are the ones that were Lagrangianized
	prop_bpvars.clear();
	prop_bprows.clear();
	if (options->lag_prop) {
		Options convert_options;
		convert_options.bp_prop_only_all = false;
		convert_options.bp_prop_only_set_packing = false;
		convert_scip_bp(scip, &convert_options, prop_bpvars, prop_bprows, &prop_skip_rows_lhs, &prop_skip_rows_rhs, false);

		int size = prop_bpvars.size();
		for (int i = 0; i < size; ++i) {
			// the above options should guarantee this; required so we can use the same mapping for propagation and instance
			assert(i == prop_bpvars[i]->solver_index);
		}

		// for (BPVar* var : prop_bpvars) {
		// 	cout << var->solver_index << " ";
		// }
		// cout << endl;
		// for (BPRow* row : prop_bprows) {
		// 	cout << *row << endl;
		// }
	}
}


DDSolver* LagrangianDDConstraintSelectorCliqueTable::create_solver(SCIP* scip, const vector<int>& var_to_subvar,
        const vector<int>& subvar_to_var, const vector<int>& fixed_vars, const vector<double>& obj, Options* options)
{
	SCIP_COL** cols;
	int ncols;

	// Recreate list of columns in subspace to retrieve clique table from SCIP
	SCIP_CALL_ABORT(SCIPgetLPColsData(scip, &cols, &ncols));
	vector<SCIP_COL*> subcols;
	for (int i = 0; i < ncols; ++i) {
		SCIP_VAR* var = SCIPcolGetVar(cols[i]);
		int idx = SCIPvarGetProbindex(var);
		if (fixed_vars[idx] == DD_UNFIXED_VAR) {
			subcols.push_back(cols[i]);
		}
	}

	// Debugging info
	// for (int i = 0; i < (int) prop_bprows.size(); ++i) {
	// 	double rhs_mod = prop_bprows[i]->rhs;
	// 	cout << "Row " << i << ": ";
	// 	for (int j = 0; j < prop_bprows[i]->nnonz; ++j) {
	// 		int ind = prop_bprows[i]->ind[j];
	// 		double coeff = prop_bprows[i]->coeffs[j];
	// 		if (fixed_vars[ind] == DD_UNFIXED_VAR) {
	// 			cout << " + " << coeff << " x" << ind;
	// 		} else {
	// 			if (fixed_vars[ind] == 1) {
	// 				rhs_mod -= coeff;
	// 			}
	// 		}
	// 	}
	// 	cout << " <= " << rhs_mod << endl;
	// }

	// Create problem
	CliqueTableInstance* inst = new CliqueTableInstance(scip, subcols.data(), (int) subcols.size(), var_to_subvar, options->lag_add_all_ct_rows);

	CliqueTablePropLinearcons* prop = NULL;
	if (!prop_bprows.empty()) {
		prop = new CliqueTablePropLinearcons(inst, prop_bpvars, prop_bprows, var_to_subvar, subvar_to_var, fixed_vars);
	}

	CliqueTableProblem* problem = new CliqueTableProblem(inst, options, prop);
	// inst->print_mapped(subvar_to_var);

	DDSolver* solver = new DDSolver(problem, options);

	if (prop != NULL) {
		solver->add_initial_node_data("ctplc", new CliqueTablePropLinearconsData(prop));
	}

	return solver;
}
