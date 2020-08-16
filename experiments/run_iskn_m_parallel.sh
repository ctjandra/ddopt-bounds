#!/bin/bash

# Simple script to launch one job per instance id (suffix).

ninsts=`ls instances/iskn_m/iskn_clique_n1000_mk0_rk150*.mps | wc -l`

for inst_id in `seq 0 $(($ninsts-1))`; do
	bash run_iskn_m.sh $inst_id&
done
