#ifndef CLIQUETABLE_PROBLEM_HPP_
#define CLIQUETABLE_PROBLEM_HPP_

#include "cliquetable_instance.hpp"
#include "cliquetable_state.hpp"
#include "../problem.hpp"
#include "ct_prop_linearcons.hpp"
#include "cliquetable_scc.hpp"

#include "cliquetable_orderings.hpp"
#include "../../core/orderings.hpp"
#include "../../core/mergers.hpp"


class CliqueTableProblem : public Problem
{
public:

	CliqueTableInstance* instance;        /**< casted instance for convenience */

	CliqueTablePropLinearcons* prop;      /**< linear propagator */

	CliqueTableProblem(CliqueTableInstance* _inst, Options* _opts, CliqueTablePropLinearcons* _prop) : Problem(_inst, _opts)
	{
		instance = static_cast<CliqueTableInstance*>(inst);
		prop = _prop;

		// Default ordering and merging
		ordering = new MinInStateCliqueTableOrdering(instance);
		merger = new MinLongestPathMerger(options->width);
	}

	CliqueTableProblem(CliqueTableInstance* _inst, Options* _opts) : CliqueTableProblem(_inst, _opts, NULL) {}

	~CliqueTableProblem()
	{
		delete prop;
	}

	CliqueTableState* create_initial_state()
	{
		IntSet intset;
		if (instance->nonnegated_only) {
			intset.resize(0, instance->nvars - 1, true);
		} else {
			intset.resize(0, (2 * instance->nvars) - 1, true);
		}

		// Nonnegated-only instances always start domain consistent
		if (!instance->nonnegated_only && options->ct_process_initial_state) {
			make_domain_consistent(instance, &intset);
		}

		return new CliqueTableState(intset);
	}

	bool cb_skip_var_for_long_arc(int var, State* state);

	void cb_layer_end(int current_var);
};


#endif /* CLIQUETABLE_PROBLEM_HPP_ */
