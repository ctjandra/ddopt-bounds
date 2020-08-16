#include "ct_prop_linearcons.hpp"


// CliqueTablePropLinearcons

void CliqueTablePropLinearcons::update_layer_end(int current_ddvar)
{
	int current_var = ddvar_to_bpvar[current_ddvar];
	BPVar* var = vars[current_var];
	assert(fixed_vars.empty() || fixed_vars[current_var] == DD_UNFIXED_VAR); // do not expect the variable to be fixed from the start
	int nrows = var->rows.size();
	for (int i = 0; i < nrows; ++i) {
		int cons = var->rows[i];
		double coeff = var->row_coeffs[i];
		if (coeff < 0) {
			minactivity_global[cons] -= coeff;
		} else {
			maxactivity_global[cons] -= coeff;
		}
	}
	processed_ddvars[current_ddvar] = true;
}


bool CliqueTablePropLinearcons::propagate(CliqueTableState* state, int bpvar, vector<double>& minactivity, vector<double>& maxactivity,
        vector<double>& rhs)
{
	// Gather all unfixed variables sharing a constraint with bpvar in order to avoid revisiting variables
	assert(fixed_vars.empty() || fixed_vars[bpvar] == DD_UNFIXED_VAR);

	int ddvar = bpvar_to_ddvar[bpvar];
	for (int bpvar_u : ddvar_neighbors[ddvar]) {
		int ddvar_u = bpvar_to_ddvar[bpvar_u];
		assert(ddvar_u >= 0); // variable must appear in decision diagram
		assert(fixed_vars.empty() || fixed_vars[bpvar_u] == DD_UNFIXED_VAR);

		if (processed_ddvars[ddvar_u]) {
			continue; // skip processed variables
		}
		if (state->get_domain(inst, ddvar_u) != DOM_ZERO_ONE) {
			continue; // skip variables with fixed domain
		}

		BPVar* var = vars[bpvar_u];
		int nrows_var = var->rows.size();

		for (int i = 0; i < nrows_var; ++i) {
			int row_idx = var->rows[i];
			double coeff = var->row_coeffs[i];

			// Try to reduce domain
			BPDomain domain = get_smallest_domain(coeff, rows[row_idx]->sense, rhs[row_idx],
			                                      minactivity[row_idx], maxactivity[row_idx]);

			if (domain != DOM_ZERO_ONE) {
				assert(domain == 0 || domain == 1);
				// cout << "Propagation: " << ddvar_u << " set to " << domain << endl;
				bool feasible = state->set_domain(inst, ddvar_u, domain);
				if (!feasible) {
					// cout << "Infeasible from propagation" << endl;
					return true; // infeasible
				}
				// One pass of propagation; could continue updating everything else here but we do not
			}
		}
	}

	return false; // feasible
}


unordered_set<int> CliqueTablePropLinearcons::create_neighbor_set(int bpvar)
{
	unordered_set<int> vars_set;
	BPVar* var = vars[bpvar];
	int nrows_var = var->rows.size();

	// Add all (subspace) variables participating in a constraint bpvar is in
	for (int i = 0; i < nrows_var; ++i) {
		int row_idx = var->rows[i];
		for (int rowv : rows[row_idx]->ind) {
			if (bpvar_to_ddvar[rowv] >= 0 && rowv != bpvar) {
				vars_set.insert(rowv);
			}
		}
	}

	return vars_set;
}


BPDomain CliqueTablePropLinearcons::get_smallest_domain(double coeff, RowSense sense, double rhs,
        double minactivity, double maxactivity)
{
	if (sense == SENSE_GE) {
		if (coeff < 0) {
			if (DBL_LT(maxactivity + coeff, rhs)) {
				return DOM_ZERO;
			}
		} else {
			if (DBL_LT(maxactivity - coeff, rhs)) {
				return DOM_ONE;
			}
		}
	} else { // sense == SENSE_LE
		if (coeff < 0) {
			if (DBL_GT(minactivity - coeff, rhs)) {
				return DOM_ONE;
			}
		} else {
			if (DBL_GT(minactivity + coeff, rhs)) {
				return DOM_ZERO;
			}
		}
	}
	return DOM_ZERO_ONE;
}


void CliqueTablePropLinearcons::update_activity_from_domain(CliqueTableState* state, vector<double>& minactivity,
        vector<double>& maxactivity, int ddvar_to_skip)
{
	int nvars = ddvar_to_bpvar.size();
	for (int ddvar = 0; ddvar < nvars; ++ddvar) {
		if (processed_ddvars[ddvar]) {
			continue; // variable already taken into account by RHSs of each state (Data)
		}
		if (ddvar == ddvar_to_skip) {
			continue;
		}
		assert(fixed_vars.empty() || fixed_vars[ddvar_to_bpvar[ddvar]] == DD_UNFIXED_VAR); // if fixed, should fail test above

		BPDomain dom = state->get_domain(inst, ddvar);

		if (dom == DOM_ZERO || dom == DOM_ONE) {
			BPVar* var = vars[ddvar_to_bpvar[ddvar]];
			int nrows = var->rows.size();
			for (int j = 0; j < nrows; ++j) {
				int cons = var->rows[j];
				double coeff = var->row_coeffs[j];

				if (coeff < 0) {
					if (dom == DOM_ONE) {
						maxactivity[cons] += coeff;
					} else {
						assert(dom == DOM_ZERO);
						minactivity[cons] -= coeff;
					}
				} else {
					if (dom == DOM_ONE) {
						minactivity[cons] += coeff;
					} else {
						assert(dom == DOM_ZERO);
						maxactivity[cons] -= coeff;
					}
				}

			}
		}
	}
}


// CliqueTablePropLinearconsData

CliqueTablePropLinearconsData::CliqueTablePropLinearconsData(CliqueTablePropLinearcons* _prop) : prop(_prop)
{
	int nrows = prop->rows.size();
	rhs.resize(nrows, 0.0);
	for (int i = 0; i < nrows; ++i) {
		rhs[i] = prop->rows[i]->rhs;
	}

	// If there are fixed variables, update RHSs
	if (!prop->fixed_vars.empty()) {
		for (int i = 0; i < nrows; ++i) {
			for (int j = 0; j < prop->rows[i]->nnonz; ++j) {
				int var = prop->rows[i]->ind[j]; // BPVar space
				// If variable is fixed, subtract corresponding value from RHS
				if (prop->fixed_vars[var] != DD_UNFIXED_VAR) {
					rhs[i] -= prop->fixed_vars[var] * prop->rows[i]->coeffs[j];
				}
			}
		}
	}
}


CliqueTablePropLinearconsData::CliqueTablePropLinearconsData(const CliqueTablePropLinearconsData& data)
{
	prop = data.prop;
	rhs = data.rhs;
	infeasible = data.infeasible;
}


NodeData* CliqueTablePropLinearconsData::transition(Problem* prob, Node* node, State* new_state, int ddvar, int val)
{
	CliqueTablePropLinearconsData* nodedata = new CliqueTablePropLinearconsData(*this);
	nodedata->transition_in_place(prob, node, new_state, ddvar, val);
	return nodedata;
}


void CliqueTablePropLinearconsData::transition_in_place(Problem* prob, Node* node, State* new_state, int ddvar, int val)
{
	CliqueTableState* state = dynamic_cast<CliqueTableState*>(new_state);
	int bpvar = prop->ddvar_to_bpvar[ddvar];
	vector<BPVar*>& vars = prop->vars;
	vector<BPRow*>& rows = prop->rows;

	if (rows.empty()) {
		return;
	}

	// Copy minactivity and maxactivity from global
	vector<double> minactivity = prop->minactivity_global;
	vector<double> maxactivity = prop->maxactivity_global;

	assert(prop->processed_ddvars[ddvar] == false);
	prop->update_activity_from_domain(state, minactivity, maxactivity, ddvar);

	assert(val == 0 || val == 1);
	BPDomain domain = (val == 0) ? DOM_ZERO : DOM_ONE;

	/* update minactivity, maxactivity, and rhs, as necessary */
	// cout << "Setting domain: bpvar " << bpvar << " to " << domain << endl;
	int nrows = vars[bpvar]->rows.size();
	for (int i = 0; i < nrows; ++i) {
		int cons = vars[bpvar]->rows[i];
		double coeff = vars[bpvar]->row_coeffs[i];

		// // Debugging info
		// cout << "Row " << cons << ": [" << minactivity[cons] << ", " << maxactivity[cons] << "] ";
		// if (rows[cons]->sense == SENSE_GE) {
		//     cout << " >= ";
		// } else {
		//     cout << " <= ";
		// }
		// cout << rhs[cons] << endl;

		// Update minactivity/maxactivity
		if (coeff < 0) {
			minactivity[cons] -= coeff;
		} else {
			maxactivity[cons] -= coeff;
		}

		// Update rhs
		if (domain == DOM_ONE) {
			rhs[cons] -= coeff;
		}

		// // Debugging info
		// cout << " to " << cons << ": [" << minactivity[cons] << ", " << maxactivity[cons] << "] ";
		// if (rows[cons]->sense == SENSE_GE) {
		//     cout << " >= ";
		// } else {
		//     cout << " <= ";
		// }
		// cout << rhs[cons] << endl;

		// Check for infeasibility
		if (rows[cons]->sense == SENSE_GE) {
			if (DBL_LT(maxactivity[cons], rhs[cons])) {
				// cout << "Infeasible: " << maxactivity[cons] << " < " << rhs[cons] << endl;
				infeasible = true;
				return;
			}
			// if (DBL_GE(prop->minactivity_global[cons], rhs[cons])) {
			//     rhs[cons] = prop->minactivity_global[cons];
			// }
		} else { // rows[cons]->sense == SENSE_LE
			if (DBL_GT(minactivity[cons], rhs[cons])) {
				// cout << "Infeasible: " << minactivity[cons] << " > " << rhs[cons] << endl;
				infeasible = true;
				return;
			}
			// if (DBL_LE(prop->maxactivity_global[cons], rhs[cons])) {
			//     rhs[cons] = prop->maxactivity_global[cons];
			// }
		}
	}

	// Run one pass of propagation
	assert(!infeasible);
	infeasible = prop->propagate(state, bpvar, minactivity, maxactivity, rhs);
}

void CliqueTablePropLinearconsData::merge(Problem* prob, NodeData* data, State* state)
{
	CliqueTablePropLinearconsData* ctdata = dynamic_cast<CliqueTablePropLinearconsData*>(data);

	// Relax the right-hand side
	for (int i = 0; i < (int) rhs.size(); ++i) {
		if ((prop->rows[i]->sense == SENSE_LE && ctdata->rhs[i] > rhs[i])
		        || (prop->rows[i]->sense == SENSE_GE && ctdata->rhs[i] < rhs[i])) {
			rhs[i] = ctdata->rhs[i];
		}
	}
}

