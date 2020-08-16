/**
 * IP model class for SCIP
 */

#ifndef MODEL_SCIP_HPP_
#define MODEL_SCIP_HPP_

#include "instance.hpp"
#include "../util/options.hpp"
#include "../bdd/bdd.hpp"
#include "scip/scip.h"

class ModelScip
{
public:

	virtual ~ModelScip() {}

	/** Create an IP model for the problem. The SCIP environment must be already created. */
	virtual SCIP_RETCODE create_ip_model(SCIP* scip) = 0;
};


#endif /* MODEL_SCIP_HPP_ */
