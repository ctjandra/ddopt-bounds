/**
 * BP model class
 */

#include "bp_model_scip.hpp"

SCIP_RETCODE BPModelScip::create_ip_model(SCIP* scip)
{
	SCIP_CALL(SCIPreadProb(scip, filename.c_str(), NULL));

	return SCIP_OKAY;
}
