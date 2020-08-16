#!/bin/bash

# Simple script to launch one job per instance.

for instance in instances/dimacs/*.clq; do
	bash run_indepset_dimacs.sh `basename $instance .clq`&
done
