#!/bin/bash

# Simple script to take only the first three instances from the instance set.
# For DIMACS, this script keeps the instance of each class that is fastest to solve:
#     brock200_2, C125.9, gen200_p0.9_55, hamming8-4, MANN_a27, p_hat300-1.

if [ -d "instances_full" ]; then
        echo "Error: instances_full directory already exists; this script might have already been run"
        exit 1
fi

mv instances instances_full
mkdir instances
mkdir instances/dimacs
mkdir instances/indepset_n150
mkdir instances/indepset_n300
mkdir instances/iskn_n
mkdir instances/iskn_m
cp instances_full/dimacs/brock200_2_complement.clq instances/dimacs
cp instances_full/dimacs/C125.9_complement.clq instances/dimacs
cp instances_full/dimacs/gen200_p0.9_55_complement.clq instances/dimacs
cp instances_full/dimacs/hamming8-4_complement.clq instances/dimacs
cp instances_full/dimacs/MANN_a27_complement.clq instances/dimacs
cp instances_full/dimacs/p_hat300-1_complement.clq instances/dimacs
cp instances_full/indepset_n150/*_{0,1,2}.* instances/indepset_n150
cp instances_full/indepset_n300/*_{0,1,2}.* instances/indepset_n300
cp instances_full/iskn_n/*_{0,1,2}.* instances/iskn_n
cp instances_full/iskn_m/*_{0,1,2}.* instances/iskn_m

echo "Done: Instances moved to instances_full and a smaller subset of instances are left in the instances directory"
