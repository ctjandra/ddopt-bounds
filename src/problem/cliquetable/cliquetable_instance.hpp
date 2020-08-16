/*
 * --------------------------------------------------------
 * Clique table instance
 * --------------------------------------------------------
 */

#ifndef CLIQUETABLE_INSTANCE_HPP_
#define CLIQUETABLE_INSTANCE_HPP_

#include <fstream>
#include <iostream>
#include <cstring>

#include "../../util/graph.hpp"
#include "../../util/intset.hpp"
#include "scip/scip.h"

#include "../instance.hpp"

using namespace std;


/**
 * Clique table instance structure
 */
class CliqueTableInstance : public Instance
{
public:
	vector<IntSet>      adj;                 /**< adjacency list */
	vector<IntSet>      adj_mask_compl;      /**< complement mask of adjacencies */
	bool                nonnegated_only;     /**< if true, only nonnegated nodes are used */


	/** Create a clique table instance for the full space of variables */
	CliqueTableInstance(SCIP* scip, bool include_ct_rows=false, bool mask_transitive=true);

	/** Create a clique table instance for the given space of variables */
	CliqueTableInstance(SCIP* scip, SCIP_COL** cols, int ncols, const vector<int>& var_to_subvar, bool include_ct_rows=false,
	                    bool mask_transitive=true);

	~CliqueTableInstance()
	{
		delete[] weights;
	}

	/** Convert the clique table into an intset vector of adjacency. Indices n+i correspond to negated versions of variable i. */
	void convert_clique_table(SCIP_COL** cols, const vector<int>& var_to_subvar);

	/** Add edges from rows that have the clique table format, even if not present in clique table */
	void convert_clique_table_rows(SCIP* scip, const vector<int>& scipvar_to_ctvar);

	/** Copy weights from SCIP */
	void extract_weights(SCIP* scip, SCIP_COL** cols, const vector<int>& var_to_subvar);

	/** Print adjacency list */
	void print();

	/** Print adjacency list given a mapping */
	void print_mapped(const vector<int>& subvar_to_var);

	/** Return number of edges of clique table (including complement edges) */
	int get_number_of_edges();

	/** Return the vertex corresponding to the negation of the given vertex */
	int get_complement(int i);

private:

	/** Create complement mask given that adj is already constructed */
	void create_complement_mask();

	/** Create complement mask given that adj is already constructed, including transitivities */
	void create_complement_mask_with_transitivities();

	/** Initialize adj */
	void init_adj(SCIP_COL** cols, const vector<int>& scipvar_to_ctvar);

	/** Update nonnegated_only attribute according to adjacency list */
	void update_nonnegated_only();
};


#endif /* CLIQUETABLE_INSTANCE_HPP_ */
