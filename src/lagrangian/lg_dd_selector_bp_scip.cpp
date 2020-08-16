#include "lg_dd_selector_bp_scip.hpp"

#include "../problem/bp/bp_reader_scip.hpp"
#include "../problem/bp/bp_problem.hpp"
#include "../problem/bp/bp_instance.hpp"
#include "../problem/bp/bp_orderings.hpp"
#include "../problem/bp/prop_linearcons.hpp"
#include "../core/mergers.hpp"
#include "../core/orderings.hpp"


DDSolver* LagrangianDDConstraintSelectorBP::create_solver(SCIP* scip, const vector<int>& var_to_subvar,
        const vector<int>& subvar_to_var, const vector<int>& fixed_vars, const vector<double>& obj, Options* options)
{
	BPInstance* inst = read_bp_instance_scip(scip, var_to_subvar, subvar_to_var);

	// for (BPRow* row : rows_in_bdd) {
	// 	cout << *row << endl;
	// }

	vector<BPProp*> props;
	if (options->lag_pure_bp_linprop) {
		props.push_back(new BPPropLinearcons(inst->rows));
	}
	BinaryProblem* problem = new BinaryProblem(inst, props, options);

	delete problem->ordering;
	problem->ordering = new CuthillMcKeePairOrdering(inst);

	DDSolver* solver = new DDSolver(problem, options);

	return solver;
}


