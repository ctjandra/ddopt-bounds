#!/bin/bash

# Run experiments on independent set + knapsack instances.
# Experiments are run sequentially; see run_iskn_n_parallel.sh for trivial parallelization.
# If passed an argument, runs only for instances with that suffix.

# Experiments run by this script:
# * Default SCIP
# * DD bound generation combining different features:
#     - Lagrangian relaxation
#     - Propagation
#     - Primal bound generation
#     - Primal pruning

testname="iskn_n"
ddopt_bin="bin/ddopt"


if [ ! -e ${ddopt_bin} ]; then
    echo "Error: Binary file ddopt does not exist in bin directory; make sure you have compiled the program"
    exit 1
fi

if [ ! -d "instances/iskn_n" ]; then
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
    instances="instances/iskn_n/*.mps"
else
    instances="instances/iskn_n/*_${1}.mps"
fi

for inst in ${instances}; do

    for seed in 1 2 3 4 5; do

    run ${ddopt_bin} "--mip-seed ${seed} --mip-time-limit 10800 --no-bounds --solver-cuts 0 ${inst}" "`basename ${inst} .mps`_nobounds-${seed}.log"

    for nvars in 100; do
        for il in 0 50; do
            for width in 100; do
                run ${ddopt_bin} "--output-stats-verbose --mip-seed ${seed} --mip-time-limit 10800 --solver-cuts 0 -w ${width} --lag-iter-limit ${il} --lag-nvars ${nvars} ${inst}" "`basename ${inst} .mps`_w${width}_v${nvars}_il${il}_nopropnopbnopp-${seed}.log"
                run ${ddopt_bin} "--output-stats-verbose --mip-seed ${seed} --mip-time-limit 10800 --solver-cuts 0 -w ${width} --lag-prop --lag-iter-limit ${il} --lag-nvars ${nvars} ${inst}" "`basename ${inst} .mps`_w${width}_v${nvars}_il${il}_withpropnopbnopp-${seed}.log"
                run ${ddopt_bin} "--output-stats-verbose --mip-seed ${seed} --mip-time-limit 10800 --lag-primal --solver-cuts 0 -w ${width} --lag-iter-limit ${il} --lag-nvars ${nvars} ${inst}" "`basename ${inst} .mps`_w${width}_v${nvars}_il${il}_nopropwithpbnopp-${seed}.log"
                run ${ddopt_bin} "--output-stats-verbose --mip-seed ${seed} --mip-time-limit 10800 --lag-primal --solver-cuts 0 -w ${width} --lag-prop --lag-iter-limit ${il} --lag-nvars ${nvars} ${inst}" "`basename ${inst} .mps`_w${width}_v${nvars}_il${il}_withpropwithpbnopp-${seed}.log"
                run ${ddopt_bin} "--output-stats-verbose --mip-seed ${seed} --mip-time-limit 10800 --lag-primal-pruning --lag-primal --solver-cuts 0 -w ${width} --lag-iter-limit ${il} --lag-nvars ${nvars} ${inst}" "`basename ${inst} .mps`_w${width}_v${nvars}_il${il}_nopropwithpbwithpp-${seed}.log"
                run ${ddopt_bin} "--output-stats-verbose --mip-seed ${seed} --mip-time-limit 10800 --lag-primal-pruning --lag-primal --solver-cuts 0 -w ${width} --lag-prop --lag-iter-limit ${il} --lag-nvars ${nvars} ${inst}" "`basename ${inst} .mps`_w${width}_v${nvars}_il${il}_withpropwithpbwithpp-${seed}.log"
            done
        done
    done

   done

done
