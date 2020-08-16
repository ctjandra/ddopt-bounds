#!/bin/bash

# Simple script to launch one job per instance id (suffix).

ninsts=`ls instances/iskn_n/iskn_clique_n300_mk30_rk150*.mps | wc -l`

for inst_id in `seq 0 $(($ninsts-1))`; do
	bash run_iskn_n.sh $inst_id&
done
