#!/bin/bash

# Run experiments on independent set + knapsack instances for gap plots.
# Experiments are run sequentially; see run_iskn_n_parallel.sh for trivial parallelization.
# If passed an argument, runs only for instances with that suffix.

# Experiments run by this script:
# * Default SCIP for 10 minutes (to obtain primal bounds)
# * Default SCIP root only
# * DD bound generation with and without propagation in Lagrangian relaxation

testname="iskn_m"
ddopt_bin="bin/ddopt"


if [ ! -e ${ddopt_bin} ]; then
    echo "Error: Binary file ddopt does not exist in bin directory; make sure you have compiled the program"
    exit 1
fi

if [ ! -d "instances/iskn_m" ]; then
    echo "Error: Instance directory not found"
    exit 1
fi


if [ ! -d "output" ]; then
    mkdir output
fi
if [ ! -d "output/output_${testname}" ]; then
    mkdir output/output_${testname}
fi

run () # $1 = binary file, $2 = arguments (including instance), $3 = complete output filename (no path)
{
    output_filename=$3
    output_path=output/output_${testname}/${output_filename}
    if [ ! -e ${output_path} ]; then
        $1 $2 > ${output_path}
    fi
}

if [ -z "$1" ]; then
    instances="instances/iskn_m/*.mps"
else
    instances="instances/iskn_m/*_${1}.mps"
fi

for inst in ${instances}; do

    for seed in 1 2 3 4 5; do

    run ${ddopt_bin} "--mip-seed ${seed} --solver-cuts 0 --no-bounds --mip-time-limit 600 ${inst}" "`basename ${inst} .mps`_full-${seed}.log"
    run ${ddopt_bin} "--mip-seed ${seed} --solver-cuts 0 --no-bounds --root-only ${inst}" "`basename ${inst} .mps`_noboundscuts-${seed}.log"
    run ${ddopt_bin} "-v --mip-seed ${seed} -w 1000 --lag-nvars 100000 --lag-iter-limit 50 --root-only --lag-initial-dd --lag-run-once ${inst}" "`basename ${inst} .mps`_w1000_ct-${seed}.log"
    run ${ddopt_bin} "-v --mip-seed ${seed} -w 1000 --lag-prop --lag-nvars 100000 --lag-iter-limit 50 --root-only --lag-initial-dd --lag-run-once ${inst}" "`basename ${inst} .mps`_w1000_ctprop-${seed}.log"

    done

done


