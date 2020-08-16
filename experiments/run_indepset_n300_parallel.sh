#!/bin/bash

# Simple script to launch one job per instance id (suffix).

ninsts=`ls instances/indepset_n300/random_300_90*.clq | wc -l`

for inst_id in `seq 0 $(($ninsts-1))`; do
	bash run_indepset_n300.sh $inst_id&
done
