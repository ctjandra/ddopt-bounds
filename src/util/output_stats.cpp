/**
 * Output statistics
 */

#include "output_stats.hpp"

#include <iostream>

using namespace std;

void print_output_stats(OutputStats* output_stats)
{
	cout << "Output statistics:" << endl;
	cout << "  Total BDD time: " << output_stats->bdd_time << endl;
	cout << "  Total bound generation time: " << output_stats->bound_time << endl;
	cout << "  Number of attempts: " << output_stats->num_attempts << endl;
	cout << "  Number of runs: " << output_stats->num_runs << endl;
	cout << "  Number of improved runs: " << output_stats->num_runs_improved << endl;
	cout << "  Number of pruned runs: " << output_stats->num_runs_pruned << endl;
	cout << "  Number of primal improvements: " << output_stats->num_primal_improved << endl;
	cout << "  Number of exact BDDs: " << output_stats->num_bdd_exact << endl;
}

void print_output_stats_extra(OutputStats* output_stats)
{
	cout << "  Number of runs by subproblem size: ";
	for (auto it : output_stats->nvars_num_runs) {
		cout << "(" << it.first << "," << it.second << ") ";
	}
	cout << endl;
	cout << "  Number of improved runs by subproblem size: ";
	for (auto it : output_stats->nvars_num_runs_improved) {
		cout << "(" << it.first << "," << it.second << ") ";
	}
	cout << endl;
	cout << "  Number of pruned runs by subproblem size: ";
	for (auto it : output_stats->nvars_num_runs_pruned) {
		cout << "(" << it.first << "," << it.second << ") ";
	}
	cout << endl;
}
