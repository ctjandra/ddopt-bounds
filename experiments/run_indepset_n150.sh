#!/bin/bash

# Run experiments on independent set instances of size 150.
# Experiments are run sequentially; see run_indepset_n150_parallel.sh for trivial parallelization.
# If passed an argument, runs only for instances with that suffix.

# Experiments run by this script:
# * Default SCIP
# * DD bound generation combining different features:
#     - Solve at subproblems of sizes 100, 200, 300
#     - Primal bound generation
#     - Primal pruning

testname="is_n150"
ddopt_bin="bin/ddopt"


if [ ! -e ${ddopt_bin} ]; then
    echo "Error: Binary file ddopt does not exist in bin directory; make sure you have compiled the program"
    exit 1
fi

if [ ! -d "instances/indepset_n150" ]; then
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
    instances="instances/indepset_n150/*.clq"
else
    instances="instances/indepset_n150/*_${1}.clq"
fi

for inst in ${instances}; do

    for seed in 1 2 3 4 5; do

    run ${ddopt_bin} "--mip-seed ${seed} --no-bounds --solver-cuts 0 ${inst}" "`basename ${inst} .clq`_nobounds-${seed}.log"

    for nvars in 50 100 150; do
        for width in 100; do
            run ${ddopt_bin} "--output-stats-verbose --mip-seed ${seed} --solver-cuts 0 -w ${width} --lag-initial-dd --lag-nvars ${nvars} ${inst}" "`basename ${inst} .clq`_w${width}_v${nvars}_simple-${seed}.log"
            run ${ddopt_bin} "--output-stats-verbose --mip-seed ${seed} --solver-cuts 0 -w ${width} --lag-initial-dd --lag-nvars ${nvars} --lag-primal-pruning ${inst}" "`basename ${inst} .clq`_w${width}_v${nvars}_pponly-${seed}.log"
            run ${ddopt_bin} "--output-stats-verbose --mip-seed ${seed} --solver-cuts 0 -w ${width} --lag-initial-dd --lag-nvars ${nvars} --lag-primal-nrp ${inst}" "`basename ${inst} .clq`_w${width}_v${nvars}_nrponly-${seed}.log"
            run ${ddopt_bin} "--output-stats-verbose --mip-seed ${seed} --solver-cuts 0 -w ${width} --lag-initial-dd --lag-nvars ${nvars} --lag-primal-nrp --lag-primal-pruning ${inst}" "`basename ${inst} .clq`_w${width}_v${nvars}_nrppp-${seed}.log"
        done
    done

    done
done
