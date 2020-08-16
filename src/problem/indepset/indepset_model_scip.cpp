/**
 * Independent set IP model class
 */

#include "indepset_model_scip.hpp"
#include "../../util/stats.hpp"


static
SCIP_RETCODE add_clique_constraints(
    SCIP*                 scip,               /**< SCIP data structure */
    Graph*                graph,              /**< graph from which to create problem */
    SCIP_VAR**            vars                /**< variables for each vertex */
)
{
	SCIP_Real* consvals;
	SCIP_VAR** consvars;
	char consname[SCIP_MAXSTRLEN];

	cout << "\n";
	cout << "-------------------------------------------" << endl;
	cout << "IP - Clique formulation" << endl;
	cout << "-------------------------------------------" << endl;
	cout << "\n";

	vector<vector<int>> cliques;
	clique_decomposition(graph, cliques);

	SCIP_CALL(SCIPallocBufferArray(scip, &consvals, graph->n_vertices));
	SCIP_CALL(SCIPallocBufferArray(scip, &consvars, graph->n_vertices));

	for (int i = 0; i < (int)cliques.size(); ++i) {
		SCIP_CONS* cons;
		int clique_size = (int)cliques[i].size();

		/* add clique constraint */
		for (int j = 0; j < clique_size; ++j) {
			int v = cliques[i][j];
			consvars[j] = vars[v];
			consvals[j] = 1;
		}

		(void) SCIPsnprintf(consname, SCIP_MAXSTRLEN, "c%d", i);
		SCIP_CALL(SCIPcreateConsBasicLinear(scip, &cons, consname, clique_size, consvars, consvals, -SCIPinfinity(scip), 1));
		SCIP_CALL(SCIPaddCons(scip, cons));
		SCIP_CALL(SCIPreleaseCons(scip, &cons));
	}

	SCIPfreeBufferArray(scip, &consvals);
	SCIPfreeBufferArray(scip, &consvars);

	return SCIP_OKAY;
}


static
SCIP_RETCODE add_edge_constraints(
    SCIP*                 scip,               /**< SCIP data structure */
    Graph*                graph,              /**< graph from which to create problem */
    SCIP_VAR**            vars                /**< variables for each vertex */
)
{
	SCIP_Real consvals[] = {1.0, 1.0};
	SCIP_VAR** consvars;
	SCIP_CONS* cons;
	char consname[SCIP_MAXSTRLEN];

	cout << "\n";
	cout << "-------------------------------------------" << endl;
	cout << "IP - Edge formulation" << endl;
	cout << "-------------------------------------------" << endl;
	cout << "\n";

	/* edge constraints */
	SCIP_CALL(SCIPallocBufferArray(scip, &consvars, 2));
	for (int i = 0; i < graph->n_vertices; ++i) {
		for (int j = i+1; j < graph->n_vertices; ++j) {
			if (graph->is_adj(i, j)) {
				consvars[0] = vars[i];
				consvars[1] = vars[j];
				(void) SCIPsnprintf(consname, SCIP_MAXSTRLEN, "e%d_%d", i, j);

				SCIP_CALL(SCIPcreateConsBasicLinear(scip, &cons, consname, 2, consvars, consvals, -SCIPinfinity(scip), 1));
				SCIP_CALL(SCIPaddCons(scip, cons));
				SCIP_CALL(SCIPreleaseCons(scip, &cons));
			}
		}
	}
	SCIPfreeBufferArray(scip, &consvars);

	return SCIP_OKAY;
}


SCIP_RETCODE IndepSetModelScip::create_ip_model(SCIP* scip)
{
	SCIP_VAR** vars;

	// Create problem
	SCIP_CALL(SCIPcreateProb(scip, "indepset", NULL, NULL, NULL, NULL, NULL, NULL, NULL));

	// Create variables and objective
	SCIP_CALL(SCIPallocBufferArray(scip, &vars, inst->graph->n_vertices));
	for (int i = 0; i < inst->graph->n_vertices; ++i) {
		char varname[SCIP_MAXSTRLEN];
		(void) SCIPsnprintf(varname, SCIP_MAXSTRLEN, "x%d", i);
		if (!indepset_options->lp_relaxation) {
			SCIP_CALL(SCIPcreateVarBasic(scip, &(vars[i]), varname, 0, 1, -1, SCIP_VARTYPE_BINARY));
		} else {
			SCIP_CALL(SCIPcreateVarBasic(scip, &(vars[i]), varname, 0, 1, -1, SCIP_VARTYPE_CONTINUOUS));
		}
		SCIP_CALL(SCIPaddVar(scip, vars[i]));
	}
	SCIP_CALL(SCIPsetObjsense(scip, SCIP_OBJSENSE_MINIMIZE));   // assume minimization

	// Add constraints
	if (indepset_options->clique_formulation) {
		SCIP_CALL(add_clique_constraints(scip, inst->graph, vars));
	} else {
		SCIP_CALL(add_edge_constraints(scip, inst->graph, vars));
	}

	SCIPfreeBufferArray(scip, &vars);

	return SCIP_OKAY;
}
