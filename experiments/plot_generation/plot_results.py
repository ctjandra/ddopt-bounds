# In summary, this script performs the following operations:
# 1. Extract relevant data from raw output files through a bash command (possibly externally through ssh)
# 2. Aggregate data and average results
# 3. Plot graphs of the results
# Running this script remotely (i.e. not on the machine containing the output files) requires the paramiko module.

################################################################################################

import collections
import numpy
import os
import sys

from scripts import data_collection
from scripts.settings import *
from scripts.plotters_indepset import *
from scripts.plotters_iskn import *


plotters = {
	'indepset_n150'     : PlotterIndepSet('random_150'),
	'indepset_n150_app' : PlotterIndepSetAppendix('random_150'),
	'indepset_n300'     : PlotterIndepSet('random_300'),
	'indepset_n300_app' : PlotterIndepSetAppendix('random_300'),
	'indepset_dimacs'   : PlotterIndepSetDimacs(),
	'iskn'              : PlotterIndepSetKnapsack(),
	'iskn_app'          : PlotterIndepSetKnapsackAppendix(),
	'iskn_gap'          : PlotterIndepSetKnapsackGap(),

	# This includes manual cleanups on the plots for instances hitting time limit.
	# Note that this package does not include densities 10%-30%, which were verified
	# to always hit time limit.
	'indepset_n300_clean': PlotterIndepSet('random_300', True),
}


def print_parameter_options():
	print
	print '    indepset_n150:     Plot independent set experiments for 150 vertices.       Requires indepset_n150 experiments.'
	print '    indepset_n150_app: Plot independent set appendix experiments for 150 vertices.   Requires indepset_n150 experiments.'
	print '    indepset_n300:     Plot independent set experiments for 300 vertices.       Requires indepset_n300 experiments.'
	print '    indepset_n300_app: Plot independent set appendix experiments for 300 vertices.   Requires indepset_n300 experiments.'
	print '    indepset_dimacs:   Plot independent set experiments for DIMACS.             Requires indepset_dimacs experiments.'
	print '    iskn:              Plot independent set + knapsack experiments on time.     Requires iskn_n experiments.'
	print '    iskn_app:          Plot independent set + knapsack appendix experiments.    Requires iskn_n experiments.'
	print '    iskn_gap:          Plot independent set + knapsack experiments on gap.      Requires iskn_m experiments.'
	print
	print '    all:               Plot all of the above.                                   Requires all of the above experiments.'
	print


def run_plotter(plotter, ssh=None):
	# Prepare parameters from plotter
	fullpath = PATH + '/' + plotter.run_directory
	selected_fields = {k : FIELDS[k] for k in plotter.required_fields if k in FIELDS}
	selected_additional_fields = {k : ADDITIONAL_FIELDS[k] for k in plotter.required_fields if k in ADDITIONAL_FIELDS}
	instance_index = INSTANCE_INDICES[plotter.instance_type]
	try: # filename filter allows us to only look at specific files from the directory
		selected_filename_filter = plotter.filename_filter
	except AttributeError:
		selected_filename_filter = None
	try:
		selected_filename_filter_per_field = plotter.filename_filter_per_field
	except AttributeError:
		selected_filename_filter_per_field = None

	# Collect data
	data, avg_data = data_collection.collect_data(selected_fields, instance_index, fullpath, USERHOST, selected_additional_fields,
		selected_filename_filter, selected_filename_filter_per_field, ssh, SHIFTED_FIELDS)

	if not os.path.exists(OUTPUT_DIR):
		os.makedirs(OUTPUT_DIR)

	# Plot data
	plotter.plot(data, avg_data, OUTPUT_DIR)


# Main script

if len(sys.argv) <= 1 or (sys.argv[1] not in plotters and sys.argv[1] != 'all'):
	print 'Parameter not supplied or invalid. Options:'
	print_parameter_options()
	sys.exit(1)

if sys.argv[1] == 'all':
	ssh = None
	if USERHOST != "":
		username, hostname = USERHOST.split('@')
		ssh = data_collection.ssh_connect(username, hostname)

	for name, plotter in sorted(plotters.iteritems()):
		if 'clean' not in name:
			print 'Running ' + name + '...'
			run_plotter(plotter, ssh)

	if ssh:
		ssh.close()
else:
	plotter = plotters[sys.argv[1]]
	run_plotter(plotter)
