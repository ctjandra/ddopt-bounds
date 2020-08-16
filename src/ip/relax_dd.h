/**
 * Decision diagram bounds for SCIP
 * @file   relax_dd.h
 * @ingroup RELAXATORS
 * @brief  dd relaxator
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_RELAX_DDBP_LAG_H__
#define __SCIP_RELAX_DDBP_LAG_H__


#include "scip/scip.h"
#include "../util/options.hpp"
#include "../util/output_stats.hpp"
#include "../problem/bp/bp_problem.hpp"
#include "../problem/bp/bp_instance.hpp"
#include "../core/mergers.hpp"
#include "../core/orderings.hpp"
#include "../lagrangian/lg_dd_selector_scip.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/** creates the dd relaxator and includes it in SCIP */
EXTERN
SCIP_RETCODE SCIPincludeRelaxDd(
    SCIP*                 scip,               /**< SCIP data structure */
    Options*              options,            /**< options */
    OutputStats*          output_stats        /**< output statistics */
);

/** Return a vector v s.t. v[i] = true iff variable with problem index i is fixed at some value */
vector<bool> get_fixed_variables(SCIP* scip);

/** Extract the set of rows to be used as Lagrangian relaxation */
SCIP_RETCODE extract_lagrangian_rows_set_packing(SCIP* scip, vector<SCIP_ROW*>& lagrangian_rows);

/** Construct decision diagram and run Lagrangian relaxation using given constraint selector */
SCIP_RETCODE construct_dd_from_bp_lag(SCIP* scip, Options* options, OutputStats* output_stats, double* dualbound,
									  SCIPRowVector* lagrangian_rows, LagrangianDDConstraintSelector* lag_selector);

/** Return objective coefficients from SCIP in a form to be used for decision diagrams */
vector<double> get_scip_objective_for_dd(SCIP* scip, SCIP_COL** cols, int ncols);

/** Return objective constant given fixed variables */
double get_scip_objective_constant_for_fixed_vars(SCIP* scip, SCIP_COL** cols, int ncols, const vector<int>& fixed_vars);

/** Count number of free (unfixed) variables */
int count_number_of_free_variables(SCIP* scip, SCIP_COL** cols, int ncols);

#ifdef __cplusplus
}
#endif

#endif
