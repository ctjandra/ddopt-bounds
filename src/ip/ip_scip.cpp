/**
 * Main IP modeling and solving
 */

#include <list>
#include <limits>
#include "ip_scip.hpp"
#include "relax_dd.h"
#include "../util/stats.hpp"
#include "../util/output_stats.hpp"

using namespace std;


SCIP_RETCODE solve_ip(ModelScip* model_builder, Options* options)
{
	Stats stats;
	stats.register_name("time");

	SCIP* scip = NULL;

	OutputStats output_stats;

	// Initialize SCIP environment
	SCIP_CALL(SCIPcreate(&scip));

	// Add bound generation system
	if (options->generate_bounds) {
		SCIP_CALL(SCIPincludeRelaxDd(scip, options, &output_stats));
	}

	// Include default plugins
	SCIP_CALL(SCIPincludeDefaultPlugins(scip));

	// Create IP model
	model_builder->create_ip_model(scip);

	stats.start_timer(0);

	// Set options
	SCIP_CALL(SCIPsetRealParam(scip, "limits/time", options->mip_time_limit));   // Time limit
	SCIP_CALL(SCIPsetIntParam(scip, "parallel/maxnthreads", 1));   // Single-threaded

	// Aggregation and restarts are not compatible with DD bounds; disabled for baseline as well
	SCIP_CALL(SCIPsetBoolParam(scip, "presolving/donotaggr", TRUE));
	SCIP_CALL(SCIPsetBoolParam(scip, "presolving/donotmultaggr", TRUE));
	SCIP_CALL(SCIPsetIntParam(scip, "presolving/maxrestarts", 0));

	// Set more options from options structure
	SCIP_CALL(set_scip_options(scip, options));

	// SCIP_CALL(SCIPprintOrigProblem(scip, NULL, NULL, FALSE));

	// Optional: Print problem to MPS file
	if (options->print_mps) {
		FILE* file = fopen("problem.mps", "w");
		SCIP_CALL(SCIPprintOrigProblem(scip, file, "mps", FALSE));
		fclose(file);
	}

	SCIP_CALL(SCIPsolve(scip));

	cout << "LP relaxation bound: " << SCIPgetFirstLPDualboundRoot(scip) << endl;

	/* Statistics */
	SCIP_CALL(SCIPprintStatistics(scip, NULL));
	SCIP_CALL(SCIPprintBestSol(scip, NULL, FALSE));

	/* Deinitialization */
	SCIP_CALL(SCIPfree(&scip));

	BMScheckEmptyMemory();

	cout << endl;
	print_output_stats(&output_stats);
	cout << endl;

	if (options->output_stats_verbose) {
		print_output_stats_extra(&output_stats);
		cout << endl;
	}

	stats.end_timer(0);

	return SCIP_OKAY;
}


/** Set SCIP options that depend on Options */
SCIP_RETCODE set_scip_options(SCIP* scip, Options* options)
{
	// MIP cuts
	assert(options->mip_cuts == -1 || options->mip_cuts == 0 || options->mip_cuts == 2);
	if (options->mip_cuts == -1) {
		SCIP_CALL(SCIPsetSeparating(scip, SCIP_PARAMSETTING_OFF, FALSE));
	} else if (options->mip_cuts == 0) {
		SCIP_CALL(SCIPsetSeparating(scip, SCIP_PARAMSETTING_DEFAULT, FALSE));
	} else if (options->mip_cuts == 2) {
		SCIP_CALL(SCIPsetSeparating(scip, SCIP_PARAMSETTING_AGGRESSIVE, FALSE));
	}

	// Root node only
	if (options->stop_after_root) {
		SCIP_CALL(SCIPsetLongintParam(scip, "limits/totalnodes", 1));
	}

	// Maximum number of rounds of cuts
	if (options->max_rounds_root >= 0) {
		SCIP_CALL(SCIPsetIntParam(scip, "separating/maxroundsroot", options->max_rounds_root));
	}

	// Random seed
	if (options->mip_seed != 0) {
		SCIP_CALL(SCIPsetIntParam(scip, "randomization/randomseedshift", options->mip_seed));
	}

	SCIP_CALL(SCIPsetIntParam(scip, "randomization/permutationseed", 100));

	return SCIP_OKAY;
}
