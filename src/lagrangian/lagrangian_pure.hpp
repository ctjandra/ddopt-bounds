#ifndef LAGRANGIAN_PURE_HPP_
#define LAGRANGIAN_PURE_HPP_

#include "scip/scip.h"
#include "../util/options.hpp"
#include "../util/util.hpp"
#include "lagrangian_cb.hpp"
#include "lg_subprob.hpp"


/** Given a SCIP model mid-solve, Lagrangianize all constraints and solve via Lagrangian relaxation */
SCIP_RETCODE solve_indepset_lagrangian_pure(SCIP* scip, Options* options);

#endif // LAGRANGIAN_PURE_HPP_