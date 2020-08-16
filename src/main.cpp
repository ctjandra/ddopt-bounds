#include <fstream>
#include <iostream>
#include <sstream>

#include "getopt.h"
#include "util/options.hpp"

#ifdef SOLVER_SCIP
#include "ip/ip_scip.hpp"
#include "problem/bp/bp_reader_scip.hpp"
#include "problem/bp/bp_model_scip.hpp"
#include "problem/indepset/indepset_model_scip.hpp"
#endif

using namespace std;


int main(int argc, char* argv[])
{

	Options options;

	if (argc < 2) {
		cout << "\nUsage: " << argv[0] << " [options] [instance file]\n";
		cout << endl;
		cout << "Decision diagram construction options:" << endl;
		cout << "    -w [width]                maximum decision diagram width (default: no limit)" << endl;
		cout << "    --no-long-arcs            do not use long arcs in the construction" << endl;
		cout << endl;
		cout << "Decision diagram bounds options:" << endl;
		cout << "    --no-bounds               do not generate bounds from DDs" << endl;
		cout << "    --lag-nvars               generate bounds from DDs at subproblems with at most this many variables" << endl;
		cout << "    --lag-nvarsfrac           generate bounds from DDs at subproblems with at most this many variables, as a fraction of total number of variables" << endl;
		cout << "    --lag-primal              generate primal bounds by checking if primal solutions generated in Lagrangian relaxation are feasible" << endl;
		cout << "    --lag-primal-nrp          generate primal bounds by finding optimal non-relaxed paths (without Lagrangian relaxation)" << endl;
		cout << "    --lag-primal-pruning      use primal bound from MIP solver to prune DDs" << endl;
		cout << "    --lag-prop                apply propagation in Lagrangian relaxation" << endl;
		cout << "    --lag-iter-limit          limit on number of iterations in Lagrangian relaxation" << endl;
		cout << "    --lag-initial-dd          prepare Lagrangian rows and decision diagrams before first LP" << endl;
		cout << "    --lag-run-once            abort at the end of first relaxation (useful to obtain bounds quickly)" << endl;
		cout << endl;
		cout << "MIP solver options:" << endl;
		cout << "    --solver-cuts [set]       MIP solver cuts: -1 none (default), 0: solver default, 2: aggressive" << endl;
		cout << "    --root-only               stop solver at the end of the root node" << endl;
		cout << "    --root-lp                 LP algorithm at root" << endl;
		cout << "    --mip-time-limit          time limit for MIP solver in seconds" << endl;
		cout << "    --mip-seed                random seed for MIP solver (does nothing if zero)" << endl;
		cout << endl;
		exit(1);
	}

	static struct option long_options[] = {
#define OPT_NO_LONG_ARCS           0
#define OPT_SOLVER_CUTS            1
#define OPT_ROOT_ONLY              2
#define OPT_ROOT_LP                3
#define OPT_NO_BOUNDS              4
#define OPT_LAG_PROP               5
#define OPT_LAG_NVARSFRAC          6
#define OPT_LAG_NVARS              7
#define OPT_LAG_TIMELIMIT          8
#define OPT_LAG_ITERLIMIT          9
#define OPT_LAG_PURE_BP           10
#define OPT_LAG_PRIMAL            11
#define OPT_LAG_VALIDATE          12
#define OPT_LAG_PRIMAL_PRUNING    13
#define OPT_LAG_DUAL_PRUNING      14
#define OPT_LAG_COMPUTE_ONLY      15
#define OPT_LAG_ADD_LINEAR        16
#define OPT_LAG_NVARS_MIN         17
#define OPT_LAG_PRIMAL_NRP        18
#define OPT_LAG_INITIAL_DD        19
#define OPT_LAG_PURE_BP_NOLINPROP 20
#define OPT_LAG_RUN_ONCE          21
#define OPT_MIP_TIME_LIMIT        22
#define OPT_MIP_SEED              23
#define OPT_OUTPUT_STATS_VERBOSE  24
		{"merger",                 required_argument, 0, 'm'},
		{"ordering",               required_argument, 0, 'o'},
		{"width",                  required_argument, 0, 'w'},
		{"verbose",                no_argument,       0, 'v'},
		{"output-stats-verbose",   no_argument,       0, OPT_OUTPUT_STATS_VERBOSE},
		{"no-long-arcs",           no_argument,       0, OPT_NO_LONG_ARCS},
		{"solver-cuts",            required_argument, 0, OPT_SOLVER_CUTS},
		{"root-only",              no_argument,       0, OPT_ROOT_ONLY},
		{"root-lp",                required_argument, 0, OPT_ROOT_LP},

		// Bound options
		{"no-bounds",              no_argument,       0, OPT_NO_BOUNDS},
		{"lag-prop",               no_argument,       0, OPT_LAG_PROP},
		{"lag-nvarsfrac",          required_argument, 0, OPT_LAG_NVARSFRAC},
		{"lag-nvars",              required_argument, 0, OPT_LAG_NVARS},
		{"lag-nvars-min",          required_argument, 0, OPT_LAG_NVARS_MIN},
		{"lag-time-limit",         required_argument, 0, OPT_LAG_TIMELIMIT},
		{"lag-primal",             no_argument,       0, OPT_LAG_PRIMAL},
		{"lag-validate",           no_argument,       0, OPT_LAG_VALIDATE},
		{"lag-primal-pruning",     no_argument,       0, OPT_LAG_PRIMAL_PRUNING},
		{"lag-iter-limit",         required_argument, 0, OPT_LAG_ITERLIMIT},
		{"lag-compute-only",       no_argument,       0, OPT_LAG_COMPUTE_ONLY},
		{"lag-pure-bp",            no_argument,       0, OPT_LAG_PURE_BP},
		{"lag-dual-pruning",       no_argument,       0, OPT_LAG_DUAL_PRUNING},
		{"lag-add-linear",         no_argument,       0, OPT_LAG_ADD_LINEAR},
		{"lag-primal-nrp",         no_argument,       0, OPT_LAG_PRIMAL_NRP},
		{"lag-initial-dd",         no_argument,       0, OPT_LAG_INITIAL_DD},
		{"lag-pure-bp-nolinprop",  no_argument,       0, OPT_LAG_PURE_BP_NOLINPROP},
		{"lag-run-once",           no_argument,       0, OPT_LAG_RUN_ONCE},

		{"mip-time-limit",         required_argument, 0, OPT_MIP_TIME_LIMIT},
		{"mip-seed",               required_argument, 0, OPT_MIP_SEED},
		{0, 0, 0, 0}
	};

	int c;
	int option_index = 0;
	while ((c = getopt_long(argc, argv, "bc:m:o:w:v", long_options, &option_index)) != -1) {
		switch (c) {
		case 'm':
			options.merge_id = atoi(optarg);
			break;
		case 'o':
			options.order_id = atoi(optarg);
			break;
		case 'w':
			options.width = atoi(optarg);
			if (options.width <= 0) {
				cout << "Error: Invalid parameter - width must be positive" << endl;
				exit(1);
			}
			break;
		case 'v':
			options.bounds_verbose = true;
			break;
		case OPT_OUTPUT_STATS_VERBOSE:
			options.output_stats_verbose = true;
			break;
		case OPT_NO_LONG_ARCS:
			options.use_long_arcs = false;
			break;
		case OPT_SOLVER_CUTS:
			options.mip_cuts = atoi(optarg);
			break;
		case OPT_ROOT_ONLY:
			options.stop_after_root = true;
			break;
		case OPT_ROOT_LP:
			options.root_lp = atoi(optarg);
			if (options.root_lp < 0) {
				cout << "Error: Invalid parameter - LP root algorithm" << endl;
				exit(1);
			}
			break;
		case OPT_NO_BOUNDS:
			options.generate_bounds = false;
			break;
		case OPT_LAG_PROP:
			options.lag_prop = true;
			break;
		case OPT_LAG_NVARSFRAC:
			options.lag_nvars_frac_to_apply = atof(optarg);
			if (options.lag_nvars_frac_to_apply < 0 || options.lag_nvars_frac_to_apply > 1) {
				cout << "Error: Invalid parameter - fraction of variables to apply DD bound" << endl;
				exit(1);
			}
			if (options.lag_nvars_to_apply >= 0) {
				cout << "Error: Cannot set both fraction of variables and number of variables to apply DD bound" << endl;
				exit(1);
			}
			break;
		case OPT_LAG_NVARS:
			options.lag_nvars_to_apply = atoi(optarg);
			if (options.lag_nvars_to_apply < 0) {
				cout << "Error: Invalid parameter - number of variables to apply DD bound" << endl;
				exit(1);
			}
			if (options.lag_nvars_frac_to_apply != 1) {
				cout << "Error: Cannot set both fraction of variables and number of variables to apply DD bound" << endl;
				exit(1);
			}
			break;
		case OPT_LAG_NVARS_MIN:
			options.lag_nvars_to_apply_min = atoi(optarg);
			if (options.lag_nvars_to_apply_min < 0) {
				cout << "Error: Invalid parameter - number of variables to apply DD bound" << endl;
				exit(1);
			}
			if (options.lag_nvars_frac_to_apply != 1) {
				cout << "Error: Cannot set both fraction of variables and number of variables to apply DD bound" << endl;
				exit(1);
			}
			break;
		case OPT_LAG_TIMELIMIT:
			options.lag_cb_time_limit = atof(optarg);
			break;
		case OPT_LAG_ITERLIMIT:
			options.lag_cb_iter_limit = atoi(optarg);
			break;
		case OPT_LAG_PURE_BP:
			options.lag_pure_bp = true;
			break;
		case OPT_LAG_PRIMAL:
			options.lag_generate_primal = true;
			break;
		case OPT_LAG_PRIMAL_NRP:
			options.lag_generate_primal_nrp = true;
			break;
		case OPT_LAG_VALIDATE:
			options.lag_validate_bounds = true;
			break;
		case OPT_LAG_PRIMAL_PRUNING:
			options.lag_primal_pruning = true;
			break;
		case OPT_LAG_DUAL_PRUNING:
			options.lag_dual_pruning = true;
			break;
		case OPT_LAG_COMPUTE_ONLY:
			options.lag_compute_only = true;
			break;
		case OPT_LAG_ADD_LINEAR:
			options.lag_add_linear = true;
			break;
		case OPT_LAG_INITIAL_DD:
			options.lag_initial_dd = true;
			break;
		case OPT_LAG_PURE_BP_NOLINPROP:
			options.lag_pure_bp_linprop = false;
			break;
		case OPT_LAG_RUN_ONCE:
			options.lag_run_once = true;
			break;
		case OPT_MIP_TIME_LIMIT:
			options.mip_time_limit = atof(optarg);
			break;
		case OPT_MIP_SEED:
			options.mip_seed = atoi(optarg);
			break;
		default:
			exit(1);
		}
	}

	// Check if input file is specified and exists
	if (optind >= argc) {
		cout << "Error: Input file not specified" << endl;
		exit(1);
	}
	ifstream input(argv[optind]);
	if (!input.good()) {
		cout << "Error: Input file cannot be opened" << endl;
		exit(1);
	}

	// Identify problem through instance file extension
	string instance_path = string(argv[optind]);
	string instance_filename = instance_path.substr(instance_path.find_last_of("\\/") + 1);
	string instance_extension = instance_filename.substr(instance_filename.find_last_of(".") + 1);
	if (instance_extension == "mps") {
#ifdef SOLVER_SCIP
		BPModelScip model_builder(instance_path);
		solve_ip(&model_builder, &options);
#else
		cout << "Error: Compilation was done without SCIP; cannot read MPS file" << endl;
#endif
	} else if (instance_extension == "clq") {
		IndepSetInstance* inst = new IndepSetInstance();
		inst->read_DIMACS(instance_path.c_str());
		cout << "\n\n*** Independent set - " << instance_filename << " ***" << endl;
#ifdef SOLVER_SCIP
		IndepSetOptions indepset_options;
		IndepSetModelScip model_builder(inst, &indepset_options);
		options.max_rounds_root = 10;
		solve_ip(&model_builder, &options);
#else
		cout << "Error: Compilation was done without SCIP; cannot run IP model" << endl;
#endif
		delete inst;
	} else {
		cout << "Error: Problem type (" << instance_extension << ") not identified" << endl;
		exit(1);
	}

	return 0;
}
