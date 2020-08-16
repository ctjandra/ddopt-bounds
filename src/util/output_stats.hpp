/**
 * Output statistics
 */

#ifndef OUTPUT_STATS_HPP_
#define OUTPUT_STATS_HPP_

#include <map>

using namespace std;

struct OutputStats {

	double bdd_time = 0.0;                /**< total time to build decision diagram */
	double bound_time = 0.0;              /**< total time spent generating bounds */

	int    num_attempts = 0;              /**< number of attempts to run relaxator; includes case when exceeds size threshold */
	int    num_runs = 0;                  /**< number of runs of the relaxator */
	int    num_runs_improved = 0;         /**< number of runs where an improved bound was found */
	int    num_runs_pruned = 0;           /**< number of runs where a node was pruned */
	int    num_bdd_exact = 0;             /**< number of runs in which decision diagram is exact */
	int    num_primal_improved = 0;       /**< number of improvements of the primal bound */

	// Warning: Synchronizing these maps with the above total values is done manually.
	// These maps are only maintained if output_stats_verbose in Options is set to true.
	map<int,int> nvars_num_runs;                  /**< number of vars to number of runs of the relaxator */
	map<int,int> nvars_num_runs_improved;         /**< number of vars to number of runs where an improved bound was found */
	map<int,int> nvars_num_runs_pruned;           /**< number of vars to number of runs where a node was pruned */
};

void print_output_stats(OutputStats* output_stats);

void print_output_stats_extra(OutputStats* output_stats);

#endif /* OUTPUT_STATS_HPP_ */
