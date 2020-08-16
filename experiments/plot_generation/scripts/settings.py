
######## User configuration ####################################################################

# For purposes of reproducing experiments, this section is the only one that needs configuration

# USERHOST must be of the form username@hostname; if empty, will access path locally
USERHOST = ""
PATH = "../output"

# Output directory
OUTPUT_DIR = 'plots'

# If true, plots are created in color
PLOT_COLOR = True


######## In-depth configuration ################################################################

# Extends a list of size below n up to n by repeating the last element; used for defining some fields below
def extend_list_by_repeating_last(lst, n):
	assert len(lst) > 0 and len(lst) <= n
	return lst + [lst[-1] for i in range(n - len(lst))]

def float_or_zero(str):
	if not str:
		return 0.0
	return float(str)

def float_negate(str):
	return -float(str)

# Bash commands that extract a value from a file (must result in a single line, rest is ignored).
# The command is piped in from cat and run directly from the shell. This implies that any command
# is accepted including pipes, but beware that this convenience turns this method *unsafe*.
# (Also, do not use single quotes and escape $'s.)
# The second value is a function applied to the resulting string (e.g. int, float, user function).
# An optional third value is True if we use cat ${f} before the command, or False if we only run the command itself
# (where ${f} would be the filename; default is True)
# Warning: If new bash commands are added, they must be tested before use; there is no error detection for them.
FIELDS = {

	'time': ('grep "Solving Time" | awk "{print \$5}"', float),
	'nodes': ('grep "Solving Nodes" | awk "{print \$4}"', int),

	'bound': ('grep "^Dual bound" | awk "{print \$3}"', float),
	'bddbound': ('grep "BDD bound (transformed):" | awk "{print \$4}"', float),
	'primalbound': ('grep "  Primal Bound" | awk "{print \$4}"', float_negate),
	'lpbound': ('grep "First LP value" | awk "{print \$5}"', float_negate),
	'lpcutsbound': ('grep "Final Dual Bound" | awk "{print \$5}"', float_negate),
	'firstlptime': ('grep "First LP Time" | awk "{print \$5}"', float),
	'bddtime': ('grep "BDD time:" | awk "{print \$3}"', float_or_zero),
	'lagtime': ('grep "Lagrangian time:" | awk "{print \$4}"', float_or_zero),

	'totalbddtime': ('grep "Total BDD time:" | awk "{print \$4}"', float),
	'totalboundtime': ('grep "Total bound generation time:" | awk "{print \$5}"', float),

  	'nruns' : ('grep "Number of runs:" | awk "{print \$4}"', int),
  	'nimproves' : ('grep "Number of improved runs:" | awk "{print \$5}"', int),
  	'nprunes' : ('grep "Number of pruned runs:" | awk "{print \$5}"', int),
  	'nprimal' : ('grep "Number of primal improvements:" | awk "{print \$5}"', int),

  	# If using verbose
  	# 'nruns' : ('grep "Run:" | wc -l', int),
  	# 'nimproves' : ('grep "[-][-] better" | wc -l', int),
  	# 'nprunes' : ('grep "[-][-] pruned" | wc -l', int),
}

def additional_field_bddlagtime(d, instance):
	lagtime = 0.0
	if 'lagtime' in d and instance in d['lagtime']:
		lagtime = d['lagtime'][instance]
	bddtime = 0.0
	if 'bddtime' in d and instance in d['bddtime']:
		bddtime = d['bddtime'][instance]
	return lagtime + bddtime

# Fields composed of collected data
ADDITIONAL_FIELDS = {
	'bddlagtime': additional_field_bddlagtime,
}

# We assume output filenames are divided by _'s. The following is the index of the part of the file name
# indicating instance number so the data can be averaged out among all instances.
# E.g. If instances are named "random_300_80_0", "random_300_80_1", "random_300_80_2", etc. then
# its INSTANCE_INDEX is 3 and results from "random_300_80_*" will be averaged.
INSTANCE_INDICES = {'indepset' : 3, 'iskn': 5, 'indepset_dimacs': None}

# Fields that should be averaged with shifted geometric mean, along with their shifts
SHIFTED_FIELDS = {
	'time': 10,
	'firstlptime': 10,
	'bddtime': 10,
	'lagtime': 10,
	'bddlagtime': 10,
	'nodes': 100,
	'totalbddtime': 10,
	'totalboundtime': 10,
}


# ######## Plot-related settings #################################################################

import matplotlib
import matplotlib.pyplot as plt
import pylab
import re

def plot_dict(d, label, marker='o', color=None, ls='-', plt=plt):
	if not any(d):
		return # do nothing if dict is empty
	it = sorted([(k, v) for k, v in d.items() if v is not None])
	plt.plot([i[0] for i in it], [i[1] for i in it], linestyle=ls, label=label, marker=marker, color=color)

def collect_plot_dict(avg_data, key_selector, key_filter, field, stdev=False, use_avg_data=True):
	if use_avg_data:
		avg_type = 0 if not stdev else 1
		return {key_selector(k) : v[field][avg_type] for k,v in avg_data.iteritems() if key_filter(k)}
	else:
		return {key_selector(k) : v[field] for k,v in avg_data.iteritems() if key_filter(k)}

def natural_sort_key(key):
	return [int(c) if c.isdigit() else c for c in re.split('([0-9]+)', key)]

if PLOT_COLOR:
	colors = ['#e41a1c', '#377eb8', '#4daf4a', '#984ea3', '#ff7f00', '#a65628', '#f781bf', '#222222']
else:
	colors = ["#000000"] * 10
markers = ['^', 'o', 's', 'v', 'D', 'x', '+', '_', '.', '*']


# ######## Filters and selectors #################################################################

# Reconstructs instance name
indepset_instance_name = lambda k : '_'.join(k[0:INSTANCE_INDICES['indepset']])
iskn_instance_name = lambda k : '_'.join(k[0:INSTANCE_INDICES['iskn']])

# Regular-expression-based filter for keys in data.
def key_filter(regexp):
	return lambda k : re.search(regexp, '_'.join(k))

key_selectors_indepset = {
	'instance_size' : (lambda k : int(k[1])),
	'density' : (lambda k : int(k[2])),
}

key_selectors_iskn = {
	'instance_size' : (lambda k : int(k[2][1:])),
	'num_knapsack' : (lambda k : int(k[3][2:])),
}
