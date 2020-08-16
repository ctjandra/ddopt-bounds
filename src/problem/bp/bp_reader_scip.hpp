/**
 * Generator of a BP instance off an MPS file using CPLEX
 */

#ifndef BP_READER_HPP_
#define BP_READER_HPP_

#include <vector>
#include <map>
#include <string>
#include "bpvar.hpp"
#include "bprow.hpp"
#include "bp_instance.hpp"
#include "cons_id.hpp"
#include "../../util/options.hpp"

#include "scip/scip.h"
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

#define NODD_TAG               "nodd" /* tag that indicates that constraint should not be added to the decision diagram 
                                       * (i.e. if the name of a constraint ends with this string, it will not be considered) */


// Note: Error checking is not robust; it is up to the user to ensure consistency
/** Class to filter out unwanted variables and constraints in problem */
class BPReaderFilter
{
public:
	virtual ~BPReaderFilter() {}

	/** Return true if variable must be excluded from problem */
	virtual bool exclude_var(string var_name) = 0;

	/** Return true if constraint must be excluded from problem */
	virtual bool exclude_cons(string cons_name) = 0;
};


/** Return vectors of BPVar and BPRow from a loaded SCIP problem */
SCIP_RETCODE convert_scip_bp(SCIP* scip, Options* options, vector<BPVar*>& vars_in_bdd, vector<BPRow*>& rows_in_bdd,
                             const vector<bool>* skip_rows_lhs=NULL, const vector<bool>* skip_rows_rhs=NULL, bool consider_fixed=true);

/** Return vectors of BPVar and BPRow from a loaded SCIP problem, using a given mapping */
SCIP_RETCODE convert_scip_bp(SCIP* scip, Options* options, vector<BPVar*>& vars_in_bdd, vector<BPRow*>& rows_in_bdd,
                             const vector<int>& var_to_subvar, const vector<int>& subvar_to_var);

/** Read instance off SCIP */
BPInstance* read_bp_instance_scip(SCIP* scip);
BPInstance* read_bp_instance_scip(SCIP* scip, const vector<int>& var_to_subvar, const vector<int>& subvar_to_var);

/** Read instance off MPS file using SCIP */
BPInstance* read_bp_instance_scip_mps(string mps_filename);


#endif // BP_READER_HPP_
