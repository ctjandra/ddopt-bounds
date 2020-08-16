#ifndef CLIQUETABLE_STATE_HPP_
#define CLIQUETABLE_STATE_HPP_

#include "../state.hpp"
#include "../problem.hpp"
#include "cliquetable_instance.hpp"
#include "../bp/bp_domains.hpp"

class CliqueTableState : public State
{
public:
	IntSet intset;
	double dual_bound;

	CliqueTableState(IntSet _intset) : intset(_intset), dual_bound(0) {}

	State* transition(Problem* prob, int var, int val);

	void merge(Problem* prob, State* rhs)
	{
		CliqueTableState* rhsi = dynamic_cast<CliqueTableState*>(rhs);
		intset.union_with(rhsi->intset);
	}

	bool equals_to(State* rhs)
	{
		CliqueTableState* rhsi = dynamic_cast<CliqueTableState*>(rhs);
		return intset.equals_to(rhsi->intset);
	}

	bool less(const State& rhs) const
	{
		const CliqueTableState& rhsi = dynamic_cast<const CliqueTableState&>(rhs);
		return intset.set < rhsi.intset.set;
	}

	int get_size()
	{
		return intset.get_size();
	}

	ostream& stream_write(ostream& os) const
	{
		os << intset;
		return os;
	}

	/** Mark a variable as already visited by removing them; used for long arcs. Assumes instance includes nonnegated nodes. */
	void mark_as_processed(int var)
	{
		int nvars = intset.get_allocated_size() / 2;
		assert(var < nvars);
		intset.remove(var);
		intset.remove(var + nvars);
	}

	BPDomain get_domain(CliqueTableInstance* inst, int var)
	{
		assert(var >= 0 && var < inst->nvars);
		bool zero = (inst->nonnegated_only || intset.contains(var+inst->nvars));
		if (intset.contains(var)) {
			if (zero) {
				return DOM_ZERO_ONE;
			} else {
				return DOM_ONE;
			}
		} else {
			if (zero) {
				return DOM_ZERO;
			} else {
				return DOM_PROCESSED;
			}
		}
	}

	/** Set domain; return true if successful, false if infeasible */
	bool set_domain(CliqueTableInstance* inst, int var, BPDomain domain)
	{
		assert(var >= 0 && var < inst->nvars);
		assert(domain == DOM_ZERO || domain == DOM_ONE);
		int nvars = inst->nvars;
		if (inst->nonnegated_only) {
			if (domain == DOM_ONE) {
				// cannot fix to one; only return infeasibility
				return intset.contains(var);
			} else {
				intset.remove(var);
				return true; // cannot claim infeasibility
			}
		} else {
			if (domain == DOM_ONE) {
				intset.remove(var + nvars);
				return intset.contains(var);
			} else {
				intset.remove(var);
				return intset.contains(var + nvars);
			}
		}
	}
};


#endif /* CLIQUETABLE_STATE_HPP_ */
