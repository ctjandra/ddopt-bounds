/**
 * Generator of a BP instance off an MPS file using CPLEX
 */

#include <cstring>
#include <algorithm>
#include "bp_reader_scip.hpp"
#include "../../util/util.hpp"


SCIP_RETCODE convert_scip_bp(SCIP* scip, Options* options, vector<BPVar*>& vars_in_bdd, vector<BPRow*>& rows_in_bdd,
                             const vector<bool>* skip_rows_lhs, const vector<bool>* skip_rows_rhs, bool consider_fixed)
{
	SCIP_ROW** rows;
	SCIP_COL** cols;
	int nrows;
	int ncols;

	SCIP_CALL(SCIPgetLPRowsData(scip, &rows, &nrows));
	SCIP_CALL(SCIPgetLPColsData(scip, &cols, &ncols));

	for (int i = 0; i < ncols; ++i) {
		if (!SCIPcolIsIntegral(cols[i])
		        || SCIPisLT(scip, SCIPcolGetLb(cols[i]), 0)
		        || SCIPisGT(scip, SCIPcolGetUb(cols[i]), 1)) {
			printf("Error: BDD relaxation only supports binary variables\n");
			return SCIP_ERROR;
		}
	}

	/* Extract relevant variables that are not yet fixed */
	vector<SCIP_VAR*> vars_in_bdd_scip;
	for (int i = 0; i < ncols; ++i) {
		SCIP_VAR* var = SCIPcolGetVar(cols[i]);
		if (!consider_fixed || !SCIPisEQ(scip, SCIPvarGetLbLocal(var), SCIPvarGetUbLocal(var))) {
			vars_in_bdd_scip.push_back(var);
		}
	}

	// SCIP always minimizes after transforming; we assume that when setting objective
	assert(SCIPgetObjsense(scip) == SCIP_OBJSENSE_MINIMIZE);

	/* Create BPVars */
	int nvars_scip = vars_in_bdd_scip.size();
	for (int i = 0; i < nvars_scip; ++i) {
		vars_in_bdd.push_back(new BPVar(vars_in_bdd_scip[i], i));
	}


	/* Extract relevant rows */

	if (options->bp_prop_only_all) {
		nrows = 0;
	}

	int nodd_tag_len = strlen(NODD_TAG);
	for (int i = 0; i < nrows; ++i)	{
		// printf(" %f <= %f + %f <= %f\n", rows[i]->lhs, rows[i]->activity, rows[i]->constant, rows[i]->rhs);
		if (SCIProwGetOrigintype(rows[i]) == SCIP_ROWORIGINTYPE_CONS) {
			/* Ignore if constraint name is tagged with the NODD_TAG */
			const char* row_name = SCIProwGetName(rows[i]);
			const char* row_type_name = SCIPconshdlrGetName(SCIProwGetOriginCons(rows[i]));
			int row_name_len = strlen(row_name);
			if (row_name_len >= nodd_tag_len && strcmp(row_name + row_name_len - nodd_tag_len, NODD_TAG) == 0) {
				continue;
			}

			/* Add rows */
			if ((skip_rows_lhs == NULL || !(*skip_rows_lhs)[i]) && SCIProwGetLhs(rows[i]) > -SCIP_DEFAULT_INFINITY) {
				BPRow* bprow = new BPRow(scip, rows[i], SENSE_GE, vars_in_bdd, row_type_name);
				if (bprow->nnonz > 0 && DBL_GT(bprow->rhs, bprow->calculate_minactivity()))	{
					if (options->bp_prop_only_set_packing && is_set_packing(bprow)) {
						assert(strcmp(row_type_name, "setppc") == 0);
						rows_in_bdd.push_back(bprow);
					} else {
						rows_in_bdd.push_back(bprow);
					}

					// cout << "Added row " << rows_in_bdd.size() - 1 << ": " << *bprow << endl;
					// cout << "-- From row ";
					// SCIPprintRow(scip, rows[i], NULL);
					// cout << endl;
				} else {
					delete bprow;
				}
			}
			if ((skip_rows_rhs == NULL || !(*skip_rows_rhs)[i]) && SCIProwGetRhs(rows[i]) < +SCIP_DEFAULT_INFINITY) {
				BPRow* bprow = new BPRow(scip, rows[i], SENSE_LE, vars_in_bdd, row_type_name);
				if (bprow->nnonz > 0 && DBL_LT(bprow->rhs, bprow->calculate_maxactivity())) {
					if (options->bp_prop_only_set_packing && is_set_packing(bprow)) {
						assert(strcmp(row_type_name, "setppc") == 0);
						rows_in_bdd.push_back(bprow);
					} else {
						rows_in_bdd.push_back(bprow);
					}

					// cout << "Added row " << rows_in_bdd.size() - 1 << ": " << *bprow << endl;
					// cout << "-- From row ";
					// SCIPprintRow(scip, rows[i], NULL);
					// cout << endl;
				} else {
					delete bprow;
				}
			}
		}
	}

	// cout << "Number of rows for BDD: " << rows_in_bdd.size() << endl;

	/* Update rows in vars */
	for (int i = 0; i < (int) vars_in_bdd.size(); ++i) {
		if (!options->bp_prop_only_set_packing && !options->bp_prop_only_all) {
			// Only initialize vars with rows that are used as states
			vars_in_bdd[i]->init_rows(rows_in_bdd);
		}
	}


	// Debugging info (cliques from clique table)
	// int ncliques = SCIPgetNCliques(scip);
	// SCIP_CLIQUE** cliques = SCIPgetCliques(scip);
	// cout << "N cliques: " << SCIPgetNCliques(scip) << endl;
	// for (int i = 0; i < ncliques; ++i) {
	// 	SCIP_VAR** vars = SCIPcliqueGetVars(cliques[i]);
	// 	SCIP_Bool* vals = SCIPcliqueGetValues(cliques[i]);
	// 	int nvars = SCIPcliqueGetNVars(cliques[i]);

	// 	// cout << "Clique " << i << ": ";
	// 	for (int u = 0; u < nvars && !SCIPisStopped(scip); ++u) {
	// 		SCIP_VAR* var;
	// 		var = (vals[u] ? vars[u] : SCIPvarGetNegatedVar(vars[u]));
	// 		cout << " " << SCIPvarGetName(var);
	// 	}
	// 	cout << endl;
	// }
	// for (int i = 0; i < ncols; ++i) {
	// 	SCIP_VAR* var = SCIPcolGetVar(cols[i]);
	// 	cout << SCIPvarGetName(var) << " " << SCIPvarGetObj(var) << endl;
	// 	cout << SCIPvarGetName(SCIPvarGetNegatedVar(var)) << " " << SCIPvarGetObj(SCIPvarGetNegatedVar(var)) << endl;
	// }

	return SCIP_OKAY;
}


/** Create BPVars and BPRows using already-constructed mapping */
SCIP_RETCODE convert_scip_bp(SCIP* scip, Options* options, vector<BPVar*>& vars_in_bdd, vector<BPRow*>& rows_in_bdd,
                             const vector<int>& var_to_subvar, const vector<int>& subvar_to_var)
{
	SCIP_ROW** rows;
	SCIP_COL** cols;
	int nrows;
	int ncols;

	SCIP_CALL(SCIPgetLPRowsData(scip, &rows, &nrows));
	SCIP_CALL(SCIPgetLPColsData(scip, &cols, &ncols));

	for (int i = 0; i < ncols; ++i) {
		if (!SCIPcolIsIntegral(cols[i])
		        || SCIPisLT(scip, SCIPcolGetLb(cols[i]), 0)
		        || SCIPisGT(scip, SCIPcolGetUb(cols[i]), 1)) {
			printf("Error: BDD relaxation only supports binary variables\n");
			return SCIP_ERROR;
		}
	}

	// SCIP always minimizes after transforming; we assume that when setting objective
	assert(SCIPgetObjsense(scip) == SCIP_OBJSENSE_MINIMIZE);

	/* Create BPVars */
	vars_in_bdd.clear();
	vars_in_bdd.resize(subvar_to_var.size(), NULL);
	assert(ncols == (int) var_to_subvar.size());
	for (int i = 0; i < ncols; ++i) {
		SCIP_VAR* var = SCIPcolGetVar(cols[i]);
		int idx = SCIPvarGetProbindex(var);
		int subidx = var_to_subvar[idx];
		if (subidx >= 0) {
			assert(subidx < (int) subvar_to_var.size());
			vars_in_bdd[subidx] = new BPVar(var, subidx);
		}
	}

	/* Extract relevant rows */

	rows_in_bdd.clear();
	int nodd_tag_len = strlen(NODD_TAG);
	for (int i = 0; i < nrows; ++i)	{
		// printf(" %f <= %f + %f <= %f\n", rows[i]->lhs, rows[i]->activity, rows[i]->constant, rows[i]->rhs);
		if (SCIProwGetOrigintype(rows[i]) == SCIP_ROWORIGINTYPE_CONS) {
			/* Ignore if constraint name is tagged with the NODD_TAG */
			const char* row_name = SCIProwGetName(rows[i]);
			const char* row_type_name = SCIPconshdlrGetName(SCIProwGetOriginCons(rows[i]));
			int row_name_len = strlen(row_name);
			if (row_name_len >= nodd_tag_len && strcmp(row_name + row_name_len - nodd_tag_len, NODD_TAG) == 0) {
				continue;
			}

			/* Add rows */
			if (SCIProwGetLhs(rows[i]) > -SCIP_DEFAULT_INFINITY) {
				BPRow* bprow = new BPRow(scip, rows[i], SENSE_GE, vars_in_bdd, row_type_name);
				if (bprow->nnonz > 0 && DBL_GT(bprow->rhs, bprow->calculate_minactivity()))	{
					rows_in_bdd.push_back(bprow);

					// cout << "Added row " << rows_in_bdd.size() - 1 << ": " << *bprow << endl;
					// cout << "-- From row ";
					// SCIPprintRow(scip, rows[i], NULL);
					// cout << endl;
				} else {
					delete bprow;
				}
			}
			if (SCIProwGetRhs(rows[i]) < +SCIP_DEFAULT_INFINITY) {
				BPRow* bprow = new BPRow(scip, rows[i], SENSE_LE, vars_in_bdd, row_type_name);
				if (bprow->nnonz > 0 && DBL_LT(bprow->rhs, bprow->calculate_maxactivity())) {
					rows_in_bdd.push_back(bprow);

					// cout << "Added row " << rows_in_bdd.size() - 1 << ": " << *bprow << endl;
					// cout << "-- From row ";
					// SCIPprintRow(scip, rows[i], NULL);
					// cout << endl;
				} else {
					delete bprow;
				}
			}
		}
	}

	cout << "Number of rows for BDD: " << rows_in_bdd.size() << endl;

	/* Update rows in vars */
	for (int i = 0; i < (int) vars_in_bdd.size(); ++i) {
		vars_in_bdd[i]->init_rows(rows_in_bdd);
	}

	return SCIP_OKAY;
}


BPInstance* read_bp_instance_scip(SCIP* scip)
{
	vector<BPVar*> vars_in_bdd;
	vector<BPRow*> rows_in_bdd;
	Options default_options;

	convert_scip_bp(scip, &default_options, vars_in_bdd, rows_in_bdd);
	BPInstance* inst = new BPInstance(vars_in_bdd, rows_in_bdd);

	return inst;
}


BPInstance* read_bp_instance_scip(SCIP* scip, const vector<int>& var_to_subvar, const vector<int>& subvar_to_var)
{
	vector<BPVar*> vars_in_bdd;
	vector<BPRow*> rows_in_bdd;
	Options default_options;

	convert_scip_bp(scip, &default_options, vars_in_bdd, rows_in_bdd, var_to_subvar, subvar_to_var);
	BPInstance* inst = new BPInstance(vars_in_bdd, rows_in_bdd);

	return inst;
}


BPInstance* read_bp_instance_scip_mps(string mps_filename)
{
	SCIP* scip = NULL;
	SCIP_CALL_ABORT(SCIPcreate(&scip));
	SCIP_CALL_ABORT(SCIPincludeDefaultPlugins(scip));
	SCIP_CALL_ABORT(SCIPreadProb(scip, mps_filename.c_str(), NULL));

	BPInstance* inst = read_bp_instance_scip(scip);

	SCIP_CALL_ABORT(SCIPfree(&scip));

	return inst;
}
