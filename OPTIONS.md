Program options
===============

This is a list of the command line arguments that can be passed to the program.

```
Usage: ./ddopt [options] [instance file]

Decision diagram construction options:
    -w [width]                maximum decision diagram width (default: no limit)
    --no-long-arcs            do not use long arcs in the construction

Decision diagram bounds options:
    --no-bounds               do not generate bounds from DDs
    --lag-nvars               generate bounds from DDs at subproblems with at most this many variables
    --lag-nvarsfrac           generate bounds from DDs at subproblems with at most this many variables, as a fraction of total number of variables
    --lag-primal              generate primal bounds by checking if primal solutions generated in Lagrangian relaxation are feasible
    --lag-primal-nrp          generate primal bounds by finding optimal non-relaxed paths (without Lagrangian relaxation)
    --lag-primal-pruning      use primal bound from MIP solver to prune DDs
    --lag-prop                apply propagation in Lagrangian relaxation
    --lag-iter-limit          limit on number of iterations in Lagrangian relaxation
    --lag-initial-dd          prepare Lagrangian rows and decision diagrams before first LP
    --lag-run-once            abort at the end of first relaxation (useful to obtain bounds quickly)

MIP solver options:
    --solver-cuts [set]       MIP solver cuts: -1 none (default), 0: solver default, 2: aggressive
    --root-only               stop solver at the end of the root node
    --root-lp                 LP algorithm at root
    --mip-time-limit          time limit for MIP solver in seconds
    --mip-seed                random seed for MIP solver (does nothing if zero)
```

The scripts in the test directory contain the command line options used for the experiments in the paper.
