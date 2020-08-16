/**
 * Main IP modeling and solving
 */

#ifndef IP_SCIP_HPP_
#define IP_SCIP_HPP_

#include "scip/scip.h"
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

#include "../bdd/bdd.hpp"
#include "../util/util.hpp"
#include "../util/stats.hpp"
#include "../util/options.hpp"
#include "../core/solver.hpp"
#include "../core/orderings.hpp"
#include "../core/mergers.hpp"
#include "../problem/model_scip.hpp"


/** Model the problem as an IP and solve it */
SCIP_RETCODE solve_ip(ModelScip* model_builder, Options* options);

/** Set SCIP options based on the provided options */
SCIP_RETCODE set_scip_options(SCIP* scip, Options* options);


#endif /* IP_SCIP_HPP_ */
