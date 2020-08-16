/**
 * Global options
 */

#ifndef OPTIONS_HPP_
#define OPTIONS_HPP_

#include <string>

using namespace std;

struct Options {

	// Options on what should be run
	bool   generate_bounds                      = true;    /**< generates bounds from DDs for IP */

	// General IP options (unrelated to DDs)
	int    mip_cuts                             = -1;      /**< setting for MIP cuts, following SCIP settings */
	bool   stop_after_root                      = false;   /**< stop solving after done with the root node */
	int    root_lp                              = -1;      /**< root LP algorithm */
	bool   print_mps                            = false;   /**< print IP to MPS file */
	int    max_rounds_root                      = -1;      /**< maximum rounds of general MIP cuts at root (-1: SCIP default) */
	double mip_time_limit                       = 3600;    /**< time limit for the MIP solver */
	int    mip_seed                             = 0;       /**< if nonzero, random seed for MIP solver */

	// DD bound options
	bool   lag_prop                             = false;   /**< propagation in Lagrangian relaxation */
	double lag_nvars_frac_to_apply              = 1;       /**< apply Lagrangian bound if number of fixed variables is at most this fraction */
	int    lag_nvars_to_apply                   = -1;      /**< apply Lagrangian bound if number of fixed variables is at most this value (disabled if negative) */
	int    lag_nvars_to_apply_min               = -1;      /**< apply Lagrangian bound if number of fixed variables is at least this value (disabled if negative) */
	double lag_cb_time_limit                    = 10;      /**< time limit for Lagrangian relaxation using ConicBundle */
	int    lag_cb_iter_limit                    = -1;      /**< oracle iteration limit for Lagrangian relaxation using ConicBundle */
	bool   lag_pure_bp                          = false;   /**< if true, run for pure binary instead of clique table */
	bool   lag_pure_bp_linprop                  = true;    /**< if true and lag_pure_bp is true, run pure binary problem with linear propagation */
	bool   lag_generate_primal                  = false;   /**< if true, generate primal bounds by checking if primal solutions generated are feasible */
	bool   lag_generate_primal_nrp              = false;   /**< if true, generate primal bounds by finding optimal non-relaxed paths */
	bool   lag_validate_bounds                  = false;   /**< if true, check if Lagrangian bounds are valid by solving sub-MIPs */
	bool   lag_primal_pruning                   = false;   /**< if true, use primal bound to prune DD */
	bool   lag_dual_pruning                     = false;   /**< if true, use dual bound (LP) to prune DD */
	bool   lag_compute_only                     = false;   /**< if true, compute DD bound without applying it (for analysis purposes) */
	bool   lag_add_all_ct_rows                  = true;    /**< if true, add all rows in canonical clique table format to DD, even if not captured by clique table */
	bool   lag_add_all_rows                     = false;   /**< if true, add all rows to Lagrangian relaxation */
	bool   lag_add_linear                       = false;   /**< if true, run heuristics to incorporate linear constraints into DD */
	bool   lag_initial_dd                       = false;   /**< if true, prepare Lagrangian rows and decision diagrams before first LP */
	bool   lag_run_once                         = false;   /**< if true, abort immediately at the end of the first relaxation */

	// DD construction
	int    order_id                             = -1;      /**< id corresponding to ordering rule */
	int    merge_id                             = -1;      /**< id corresponding to merging rule */
	int    width                                = -1;      /**< width limit for relaxation (-1 means unrestricted) */
	bool   use_long_arcs                        = true;    /**< whether long arcs should be used in the DD */
	string fixed_order_filename                 = "fixed_order.txt";  /**< input file for a fixed order for the DD */
	double order_rand_min_state_prob            = 0.8;     /**< probability for the randomized min in state ordering */
	bool   delete_old_states                    = true;    /**< free states from nodes of previous layers to reduce memory usage */

	// BP options
	bool   bp_prop_only_set_packing             = false;   /**< does not add set packing constraints as RHSs in state; instead, propagate them only */
	bool   bp_prop_only_all                     = false;   /**< does not add any constraints as RHSs in state; instead, propagate them only */

	// Clique table options
	bool   ct_process_initial_state             = true;    /**< ensure that initial state is domain consistent */

	// Output options
	bool   quiet                                = false;   /**< do not output DD construction information */
	bool   bounds_verbose                       = false;   /**< print information on bounds */
	bool   output_stats_verbose                 = false;   /**< print a verbose version of output_stats */

};


#endif /* OPTIONS_HPP_ */
