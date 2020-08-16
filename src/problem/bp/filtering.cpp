#include "filtering.hpp"

void BPFiltering::filter(BPRow* row)
{
	// Run top-down and bottom-up pass with left-hand side of row
	BPOptimizePassFunc* filter_func;
	if (row->sense == SENSE_LE) {
		filter_func = new BPMinimizePassFunc(row, bpvar_to_ddvar);
	} else { // row->sense == SENSE_GE
		filter_func = new BPMaximizePassFunc(row, bpvar_to_ddvar);
	}
	bdd_pass(bdd, filter_func, filter_func);

	// Calculate the feasibility of each arc with respect to row
	int bdd_size = bdd->layers.size();
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = bdd->layers[layer].size();

		for (int k = 0; k < size; ++k) {
			Node* parent = bdd->layers[layer][k];
			double parent_val = boost::any_cast<BDDPassValues*>(parent->temp_data)->top_down_val; // parent (source)

			for (int val = 0; val <= 1; ++val) {
				Node* child = (val == 0) ? parent->zero_arc : parent->one_arc;

				if (child != NULL) {
					double child_val = boost::any_cast<BDDPassValues*>(child->temp_data)->bottom_up_val;
					double row_arc_val = val * filter_func->coeffs[bdd->layer_to_var[layer]];
					// Long arcs are ok as long as they are of the form (*,0,...0)
					double arc_pass_val = parent_val + child_val + row_arc_val;

					// cout << "[" << layer << ", " << k << ", 0]" << " vals " << parent_val << ", " << row_arc_val << ", " << child_val;
					// cout << " / Arc val: " << arc_pass_val << " / RHS: " << row->rhs << endl;

					if ((row->sense == SENSE_LE && DBL_GE(arc_pass_val, row->rhs))
					        || (row->sense == SENSE_GE && DBL_LE(arc_pass_val, row->rhs))) {
						// Arc is infeasible with respect to row: filter
						cout << "Filtered arc above: value " << arc_pass_val << " against rhs " << row->rhs << endl;
						parent->detach_arc(val);
					}
				}
			}
		}
	}

	// Clean up values from pass
	bdd_pass_clean_up(bdd);

	delete filter_func;
}


void BPFiltering::filter(vector<BPRow*>& rows)
{
	int nrows = rows.size();
	for (int i = 0; i < nrows; ++i) {
		cout << "Filtering row " << i << endl;
		filter(rows[i]);
	}
}


void BPFiltering::print_splittable_nodes(BPRow* row)
{
	// Run top-down and bottom-up pass with left-hand side of row
	BPOptimizePassFunc* filter_func;
	if (row->sense == SENSE_LE) {
		filter_func = new BPMaximizePassFunc(row, bpvar_to_ddvar);
	} else { // row->sense == SENSE_GE
		filter_func = new BPMinimizePassFunc(row, bpvar_to_ddvar);
	}
	bdd_pass(bdd, filter_func, filter_func);

	cout << "RHS: " << row->rhs << endl;

	int bdd_size = bdd->layers.size();
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = bdd->layers[layer].size();

		cout << "Layer " << layer << ":  ";

		for (int k = 0; k < size; ++k) {
			Node* node = bdd->layers[layer][k];
			double top_down_val = boost::any_cast<BDDPassValues*>(node->temp_data)->top_down_val;
			double bottom_up_val = boost::any_cast<BDDPassValues*>(node->temp_data)->bottom_up_val;
			double node_val = top_down_val + bottom_up_val;

			if ((row->sense == SENSE_LE && DBL_LE(node_val, row->rhs))
			        || (row->sense == SENSE_GE && DBL_GE(node_val, row->rhs))) {
				// Node is unrefinable; i.e. all paths through this node are feasible
				cout << k << " ";
			} else {
				cout << "[" << k << "] ";
			}
			cout << node_val << " ";
		}
		cout << endl;
	}

	// Clean up values from pass
	bdd_pass_clean_up(bdd);

	delete filter_func;



	if (row->sense == SENSE_LE) {
		filter_func = new BPMinimizePassFunc(row, bpvar_to_ddvar);
	} else { // row->sense == SENSE_GE
		filter_func = new BPMaximizePassFunc(row, bpvar_to_ddvar);
	}
	bdd_pass(bdd, filter_func, filter_func);

	cout << "RHS: " << row->rhs << endl;

	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = bdd->layers[layer].size();

		cout << "Layer " << layer << ":  ";

		for (int k = 0; k < size; ++k) {
			Node* node = bdd->layers[layer][k];
			double top_down_val = boost::any_cast<BDDPassValues*>(node->temp_data)->top_down_val;
			double bottom_up_val = boost::any_cast<BDDPassValues*>(node->temp_data)->bottom_up_val;
			double node_val = top_down_val + bottom_up_val;
			cout << node_val << " ";
		}
		cout << endl;
	}

	// Clean up values from pass
	bdd_pass_clean_up(bdd);

	delete filter_func;
}


void BPFiltering::print_splittable_nodes(vector<BPRow*>& rows)
{
	int nrows = rows.size();
	for (int i = 0; i < nrows; ++i) {
		cout << "Row " << i << endl;
		print_splittable_nodes(rows[i]);
	}
}
