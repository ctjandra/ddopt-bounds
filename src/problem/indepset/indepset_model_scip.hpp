/**
 * Independent set IP model class
 */

#ifndef INDEPSET_MODEL_SCIP_HPP_
#define INDEPSET_MODEL_SCIP_HPP_

#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

#include "scip/scip.h"
#include "indepset_instance.hpp"
#include "indepset_options.hpp"
#include "../model_scip.hpp"
#include "../../util/options.hpp"

class IndepSetModelScip : public ModelScip
{
public:
	IndepSetInstance* inst;
	IndepSetOptions* indepset_options;

	IndepSetModelScip(IndepSetInstance* _inst, IndepSetOptions* _indepset_options) : inst(_inst),
		indepset_options(_indepset_options) {}

	SCIP_RETCODE create_ip_model(SCIP* scip);
};


#endif /* INDEPSET_MODEL_SCIP_HPP_ */
