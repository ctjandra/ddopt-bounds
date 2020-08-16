/**
 * BP model class
 */

#ifndef BP_MODEL_SCIP_HPP_
#define BP_MODEL_SCIP_HPP_

#include <string>
#include "../model_scip.hpp"

class BPModelScip : public ModelScip
{
public:
	string filename;

	BPModelScip(string _filename) : filename(_filename) {}

	SCIP_RETCODE create_ip_model(SCIP* scip);
};


#endif /* BP_MODEL_SCIP_HPP_ */
