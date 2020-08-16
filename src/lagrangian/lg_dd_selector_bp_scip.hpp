/** Binary problem strategy for decision diagram (no Lagrangian relaxation bounds) */

#ifndef LG_DD_SELECTOR_BP_SCIP_HPP_
#define LG_DD_SELECTOR_BP_SCIP_HPP_

#include "scip/scip.h"
#include "lg_dd_selector_scip.hpp"
#include "../problem/bp/bprow.hpp"
#include "../problem/bp/bpvar.hpp"
#include "../problem/bp/filtering.hpp"


class LagrangianDDConstraintSelectorBP : public LagrangianDDConstraintSelector
{
public:

	~LagrangianDDConstraintSelectorBP()	{}

	// Functions from parent

	bool exists_structure(SCIP* scip, SCIP_ROW** rows, int nrows)
	{
		return true;    // Structure always exists
	}

	SCIPRowVector* extract_lagrangian_rows(SCIP* scip, SCIP_ROW** rows, int nrows, Options* options) // Nothing to Lagrangianize
	{
		return new SCIPRowVector();
	}

	void prepare_dd_construction(SCIP* scip, SCIP_ROW** rows, int nrows, Options* options) {} // Nothing to prepare

	void preprocess_fixed_vars(SCIP* scip, vector<int>& fixed_vars) {} // No preprocessing

	void create_mappings(SCIP* scip, const vector<int>& fixed_vars, vector<int>& var_to_subvar, vector<int>& subvar_to_var)
	{
		create_mappings_default(scip, fixed_vars, var_to_subvar, subvar_to_var);
	}

	DDSolver* create_solver(SCIP* scip, const vector<int>& var_to_subvar, const vector<int>& subvar_to_var,
	                        const vector<int>& fixed_vars, const vector<double>& obj, Options* options);

};


#endif // LG_DD_SELECTOR_BP_SCIP_HPP_