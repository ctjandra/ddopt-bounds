#ifndef CT_PROP_LINEARCONS_HPP_
#define CT_PROP_LINEARCONS_HPP_

#include <unordered_set>
#include <vector>
#include "../bp/bp_domains.hpp"
#include "../bp/bprow.hpp"
#include "../bp/bpvar.hpp"
#include "cliquetable_state.hpp"
#include "../../bdd/nodedata.hpp"
#include "../../util/util.hpp"
#include "../../lagrangian/lg_dd_selector_scip.hpp"

using namespace std;


class CliqueTablePropLinearcons
{
public:
	CliqueTableInstance* inst;            /**< instance */

	vector<BPVar*> vars;
	vector<BPRow*> rows;                  /**< rows that need to be propagated */

	vector<double> minactivity_global;    /**< minactivity updated at each layer */
	vector<double> maxactivity_global;    /**< maxactivity updated at each layer */
	vector<bool> processed_ddvars;        /**< marker for variables already processed by DD (in subspace); equivalent to those
                                            *  with domain DOM_PROCESSED when instance is not using nonnegated_only */

	// Structures used if working on a restricted subspace
	const vector<int>& bpvar_to_ddvar;   /**< mapping from index from vars (BPVar) to index from DD (subspace) */
	const vector<int>& ddvar_to_bpvar;   /**< mapping from index from DD (subspace) to index from vars (BPVar) */
	const vector<int>& fixed_vars;       /**< values to which variables outside subspace (in BPVar space) are fixed
                                           *  (empty means unused) */

	vector<unordered_set<int>> ddvar_neighbors; /**< neighbors of variables in rows (in subspace) */


	CliqueTablePropLinearcons(CliqueTableInstance* _inst, vector<BPVar*>& _vars, vector<BPRow*>& _rows,
	                          const vector<int>& _bpvar_to_ddvar, const vector<int>& _ddvar_to_bpvar, const vector<int>& _fixed_vars) :
		inst(_inst), vars(_vars), rows(_rows), processed_ddvars(_ddvar_to_bpvar.size(), false),
		bpvar_to_ddvar(_bpvar_to_ddvar), ddvar_to_bpvar(_ddvar_to_bpvar), fixed_vars(_fixed_vars)
	{
		int nrows = rows.size();
		minactivity_global.resize(nrows, 0.0);
		maxactivity_global.resize(nrows, 0.0);
		for (int i = 0; i < nrows; ++i) {
			for (int j = 0; j < rows[i]->nnonz; ++j) {
				if (fixed_vars.empty() || fixed_vars[rows[i]->ind[j]] == DD_UNFIXED_VAR) {
					double coeff = rows[i]->coeffs[j];
					minactivity_global[i] += MIN(0, coeff); /* minimum between possible evaluations of term a_k * x_k */
					maxactivity_global[i] += MAX(0, coeff); /* maximum between possible evaluations of term a_k * x_k */
				}
			}
		}
		assert(bpvar_to_ddvar.size() == vars.size());

		// Prepare neighbors of variables
		ddvar_neighbors.reserve(ddvar_to_bpvar.size());
		for (int bpvar : ddvar_to_bpvar) {
			ddvar_neighbors.push_back(create_neighbor_set(bpvar));
		}
	}

	CliqueTablePropLinearcons(CliqueTableInstance* _inst, vector<BPVar*>& _vars, vector<BPRow*>& _rows) :
		CliqueTablePropLinearcons(_inst, _vars, _rows, full_mapping(rows.size()), full_mapping(rows.size()), {}) {}

	bool propagate(CliqueTableState* state, int bpvar, vector<double>& minactivity, vector<double>& maxactivity,
	               vector<double>& rhs);

	void update_layer_end(int current_ddvar);

	void update_activity_from_domain(CliqueTableState* state, vector<double>& minactivity, vector<double>& maxactivity,
	                                 int ddvar_to_skip);

private:
	/** Return all variables participating in the constraints bpvar is in. */
	unordered_set<int> create_neighbor_set(int bpvar);

	/** Return the smallest domain for the given variable w.r.t. a single constraint, assuming domain is not yet set */
	BPDomain get_smallest_domain(double coeff, RowSense sense, double rhs, double minactivity, double maxactivity);

	bool is_alwaysfeasible(RowSense sense, double rhs, double minactivity, double maxactivity);

	vector<int> full_mapping(int n)
	{
		vector<int> mapping(n);
		for (int i = 0; i < n; ++i) {
			mapping[i] = i;
		}
		return mapping;
	}

};


class CliqueTablePropLinearconsData : public NodeData
{
public:
	CliqueTablePropLinearcons* prop;
	vector<double> rhs;

	/** Constructor for initial data */
	CliqueTablePropLinearconsData(CliqueTablePropLinearcons* _prop);

	/** Copy constructor */
	CliqueTablePropLinearconsData(const CliqueTablePropLinearconsData& data);

	/** Transition function (in-place, modifies data) */
	void transition_in_place(Problem* prob, Node* node, State* new_state, int ddvar, int val);

	NodeData* transition(Problem* prob, Node* node, State* new_state, int ddvar, int val);

	/** Merge with another data */
	void merge(Problem* prob, NodeData* rhs, State* state);

};


#endif // CT_PROP_LINEARCONS_HPP_
