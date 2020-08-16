#ifndef FILTERING_HPP_
#define FILTERING_HPP_

#include <unordered_map>
#include "../../bdd/bdd.hpp"
#include "../../bdd/bdd_pass.hpp"
#include "bprow.hpp"

using namespace std;

/** Given a linear constraint, removes all arcs from a BDD violating the constraint. */
class BPFiltering
{
	BDD* bdd;
	const vector<int>& bpvar_to_ddvar;

public:
	BPFiltering(BDD* _bdd, const vector<int>& _bpvar_to_ddvar) : bdd(_bdd), bpvar_to_ddvar(_bpvar_to_ddvar) {}

	void filter(BPRow* row);

	void filter(vector<BPRow*>& rows);

	void print_splittable_nodes(BPRow* row);
	void print_splittable_nodes(vector<BPRow*>& rows);
};


/**
 * Auxiliary class to do top-down/bottom-up passes in order to filter arcs.
 * Calculates the minimum or maximum left-hand side of all partial solutions or completions, as determined by
 * subclass implementation.
 */
class BPOptimizePassFunc : public BDDPassFunc
{
protected:

	BPRow* row;                           /**< row to filter */

public:

	unordered_map<int,double> coeffs;     /**< map from variable to coefficient in row, for convenience */

	BPOptimizePassFunc(BPRow* _row) : row(_row)
	{
		for (int i = 0; i < _row->nnonz; ++i) {
			coeffs[_row->ind[i]] = _row->coeffs[i];
		}
	}

	BPOptimizePassFunc(BPRow* _row, const vector<int>& bpvar_to_ddvar) : row(_row)
	{
		for (int i = 0; i < _row->nnonz; ++i) {
			coeffs[bpvar_to_ddvar[_row->ind[i]]] = _row->coeffs[i];
		}
	}

	double start_val()
	{
		return 0;
	}

	/** Return initial value at the start of the pass */
	virtual double init_val() = 0;

	/** Return best between two values according to pass */
	virtual double select_best(double target_val, double new_val) = 0;

	double apply(int layer, int var, int arc_val, double source_val, double target_val, Node* source, Node* target)
	{
		double new_val = source_val;
		if (arc_val == 1) {
			// Add coefficient if nonzero
			unordered_map<int,double>::iterator it = coeffs.find(var);
			if (it != coeffs.end()) {
				new_val += it->second;
			}
		}
		return select_best(target_val, new_val);
	}
};

/** Calculates the minimum left-hand side of all partial solutions or completions */
class BPMinimizePassFunc : public BPOptimizePassFunc
{
public:

	BPMinimizePassFunc(BPRow* _row) : BPOptimizePassFunc(_row) {}
	BPMinimizePassFunc(BPRow* _row, const vector<int>& bpvar_to_ddvar) : BPOptimizePassFunc(_row, bpvar_to_ddvar) {}

	double init_val()
	{
		return +numeric_limits<double>::infinity(); // minimize left-hand side
	}

	double select_best(double target_val, double new_val)
	{
		return MIN(target_val, new_val);
	}
};


/** Calculates the maximum left-hand side of all partial solutions or completions */
class BPMaximizePassFunc : public BPOptimizePassFunc
{
public:

	BPMaximizePassFunc(BPRow* _row) : BPOptimizePassFunc(_row) {}
	BPMaximizePassFunc(BPRow* _row, const vector<int>& bpvar_to_ddvar) : BPOptimizePassFunc(_row, bpvar_to_ddvar) {}

	double init_val()
	{
		return -numeric_limits<double>::infinity(); // maximize left-hand side
	}

	double select_best(double target_val, double new_val)
	{
		return MAX(target_val, new_val);
	}
};


#endif // FILTERING_HPP_
