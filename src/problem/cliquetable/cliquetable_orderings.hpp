/*
 * --------------------------------------------------------
 * Ordering class
 * --------------------------------------------------------
 */

#ifndef CLIQUETABLE_ORDERINGS_HPP_
#define CLIQUETABLE_ORDERINGS_HPP_

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/discrete_distribution.hpp>

#include "../../core/order.hpp"
#include "cliquetable_instance.hpp"
#include "cliquetable_state.hpp"

using namespace std;


// Minimum degree ordering for clique table
struct MinDegreeCliqueTableOrdering : Ordering {

	CliqueTableInstance* inst;
	vector<int> v_in_layer;   // var at each layer

	MinDegreeCliqueTableOrdering(CliqueTableInstance* _inst) : inst(_inst)
	{
		sprintf(name, "mindegree");
		construct_ordering();
	}

	int select_next_var(int layer)
	{
		assert(layer >= 0 && layer < inst->nvars);
		return v_in_layer[layer];
	}

private:
	void construct_ordering();
};


/** Select variable that is free in the least number of states (or alternatively fixed in most states) */
struct MinInStateCliqueTableOrdering : Ordering {

	CliqueTableInstance* inst;
	vector<int> in_state_counter;   /**< number of states containing each variable */
	vector<bool> processed_vars;    /**< indicates which variables have been selected */

	MinInStateCliqueTableOrdering(CliqueTableInstance* _inst) : inst(_inst), in_state_counter(_inst->nvars),
		processed_vars(_inst->nvars, false)
	{
		sprintf(name, "min_in_state");
	}

	int select_next_var(int layer);

	void cb_state_created(State* state);
	void cb_state_removed(State* state);
};

#endif
