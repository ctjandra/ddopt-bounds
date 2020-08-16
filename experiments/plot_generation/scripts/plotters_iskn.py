import collections
import numpy
import re
import itertools
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import data_collection

from settings import *


class PlotterIndepSetKnapsack(object):
	"""Plotter for Figures 10 and 11, and Tables 2 and 3"""

	instance_type = 'iskn'
	run_directory = 'output_iskn_n'
	required_fields = ['time', 'nodes']
	filename_filter = ''
	filename_filter_per_field = None

	def plot(self, data, avg_data, output_dir):
		self.plot_iskn_times(data, avg_data, output_dir)
		self.plot_iskn_nodes(data, avg_data, output_dir)
		self.plot_iskn_times_scatter(data, avg_data, output_dir)
		self.plot_iskn_nodes_scatter(data, avg_data, output_dir)
		self.write_table_speedups(data, avg_data, output_dir)
		self.write_table_features(data, avg_data, output_dir)

	def plot_iskn_times(self, data, avg_data, output_dir):
		times_mip_dd = collect_plot_dict(avg_data, key_selectors_iskn['instance_size'], key_filter('w100_v100_il50_withpropwithpbwithpp'), 'time')
		times_mip_only = collect_plot_dict(avg_data, key_selectors_iskn['instance_size'], key_filter('nobounds'), 'time')

		plt.clf()
		plot_dict(times_mip_dd, 'MIP + DD bounds', color=colors[1], marker='o', ls='-')
		plot_dict(times_mip_only, 'MIP', color='black', marker='.', ls='--')
		plt.xlabel('Instance size')
		plt.ylabel('Total solving time (s)')
		plt.xlim(300, 450)
		plt.legend(loc=0, fontsize=12)
		plt.savefig(output_dir + '/' + 'fig10a_iskn_bounds_times_overall.pdf')

	def plot_iskn_nodes(self, data, avg_data, output_dir):
		nodes_mip_dd = collect_plot_dict(avg_data, key_selectors_iskn['instance_size'], key_filter('w100_v100_il50_withpropwithpbwithpp'), 'nodes')
		nodes_mip_only = collect_plot_dict(avg_data, key_selectors_iskn['instance_size'], key_filter('nobounds'), 'nodes')

		plt.clf()
		plot_dict(nodes_mip_dd, 'MIP + DD bounds', color=colors[1], marker='o', ls='-')
		plot_dict(nodes_mip_only, 'MIP', color='black', marker='.', ls='--')
		plt.xlabel('Instance size')
		plt.ylabel('Number of nodes')
		plt.yscale('log')
		plt.xlim(300, 450)
		plt.legend(loc=0, fontsize=12)
		plt.savefig(output_dir + '/' + 'fig10b_iskn_bounds_nodes_overall.pdf')

	def flatten(self, d):
		flat = {}
		for k1, v1 in d.iteritems():
			for k2, v2 in v1.iteritems():
				flat[(k1,k2)] = v2
		return flat

	def plot_iskn_times_scatter(self, data, avg_data, output_dir):
		times_mip_dd = collect_plot_dict(data, key_selectors_iskn['instance_size'], key_filter('w100_v100_il50_withpropwithpbwithpp'),
			'time', use_avg_data=False)
		times_mip_only = collect_plot_dict(data, key_selectors_iskn['instance_size'], key_filter('nobounds'), 'time', use_avg_data=False)
		times_mip_dd_flat = self.flatten(times_mip_dd)
		times_mip_only_flat = self.flatten(times_mip_only)

		plt.clf()
		plt.figure(figsize=(6,6))
		scatter_plot = [(times_mip_only_flat[k], v) for k, v in times_mip_dd_flat.iteritems()]
		sx = [v[0] for v in scatter_plot]
		sy = [v[1] for v in scatter_plot]
		plt.xscale('log', basex=10)
		plt.yscale('log', basey=10)
		plt.scatter(sx, sy, s=[24] * len(sx), color=colors[1], linewidth=1)
		lower_lim = 50
		upper_lim = 8000
		plt.plot([lower_lim, upper_lim], [lower_lim, upper_lim], color='black')
		plt.xlim(lower_lim, upper_lim)
		plt.ylim(lower_lim, upper_lim)
		plt.xlabel('Solving time for MIP (s)')
		plt.ylabel('Solving time for MIP + DD bounds (s)')
		plt.savefig(output_dir + '/' + 'fig11a_iskn_bounds_n_scatter_time.pdf')

	def plot_iskn_nodes_scatter(self, data, avg_data, output_dir):
		nodes_mip_dd = collect_plot_dict(data, key_selectors_iskn['instance_size'], key_filter('w100_v100_il50_withpropwithpbwithpp'),
			'nodes', use_avg_data=False)
		nodes_mip_only = collect_plot_dict(data, key_selectors_iskn['instance_size'], key_filter('nobounds'), 'nodes', use_avg_data=False)
		nodes_mip_dd_flat = self.flatten(nodes_mip_dd)
		nodes_mip_only_flat = self.flatten(nodes_mip_only)

		plt.clf()
		plt.figure(figsize=(6,6))
		scatter_plot = [(nodes_mip_only_flat[k], v) for k, v in nodes_mip_dd_flat.iteritems()]
		sx = [v[0] for v in scatter_plot]
		sy = [v[1] for v in scatter_plot]
		plt.xscale('log', basex=10)
		plt.yscale('log', basey=10)
		plt.scatter(sx, sy, s=[24] * len(sx), color=colors[1], linewidth=1)
		lower_lim = 150
		upper_lim = 250000
		plt.plot([lower_lim, upper_lim], [lower_lim, upper_lim], color='black')
		plt.xlim(lower_lim, upper_lim)
		plt.ylim(lower_lim, upper_lim)
		plt.xlabel('Number of nodes for MIP')
		plt.ylabel('Number of nodes for MIP + DD bounds')
		plt.savefig(output_dir + '/' + 'fig11b_iskn_bounds_n_scatter_nodes.pdf')

	def write_table_speedups(self, data, avg_data, output_dir):
		baseline_time_plot = collect_plot_dict(avg_data, key_selectors_iskn['instance_size'], key_filter('nobounds'), 'time')
		time_plot = collect_plot_dict(avg_data, key_selectors_iskn['instance_size'], key_filter('w100_v100_il50_withpropwithpbwithpp'), 'time')
		speedup_plots = {x : 100 * (baseline_time_plot[x] / time_plot[x] - 1) for x in time_plot.keys()}

		baseline_nodes_plot = collect_plot_dict(avg_data, key_selectors_iskn['instance_size'], key_filter('nobounds'), 'nodes')
		nodes_plot = collect_plot_dict(avg_data, key_selectors_iskn['instance_size'], key_filter('w100_v100_il50_withpropwithpbwithpp'), 'nodes')
		nodesred_plots = {x : 100 * (1 - nodes_plot[x] / baseline_nodes_plot[x]) for x in nodes_plot.keys()}

		f = open(output_dir + '/' + 'tab2_iskn_speedups.txt', 'w')
		f.write("Speed-up (%)\n")
		for k in sorted(speedup_plots.keys()):
			f.write((str(int(k)) + '\t' + str(round(speedup_plots[k], 2))).expandtabs(40) + '\n')
		f.write('\n')
		f.write("Node reduction (%)\n")
		for k in sorted(nodesred_plots.keys()):
			f.write((str(int(k)) + '\t' + str(round(nodesred_plots[k], 2))).expandtabs(40) + '\n')
		f.write('\n')
		f.close()

	def average_all(self, data, filter_key, field, shift):
		vals = []
		for k, v in data.iteritems():
			if filter_key(k):
				vals += v[field].values()
		return data_collection.shifted_geo_mean(vals, shift)

	def write_table_features(self, data, avg_data, output_dir):
		keys = ['w100_v100_il50_withpropwithpbwithpp',
			'w100_v100_il0_withpropwithpbwithpp',
			'w100_v100_il50_nopropwithpbwithpp',
			'w100_v100_il0_nopropwithpbwithpp',
			'w100_v100_il50_withpropwithpbnopp',
			'w100_v100_il50_withpropnopbwithpp',
			'w100_v100_il50_withpropnopbnopp',
			'w100_v100_il0_nopropnopbnopp']
		labels = ['All', 'No Lagrangian', 'No propagation', 'No Lagrangian or propagation', 'No primal pruning',
			'No primal heuristic', 'No primal pruning or heuristic', 'Dual bounds only']

		time_shift = 10
		nodes_shift = 100
		speedups = []
		nodesreds = []
		baseline_time = self.average_all(data, key_filter('nobounds'), 'time', time_shift)
		baseline_nodes = self.average_all(data, key_filter('nobounds'), 'nodes', nodes_shift)
		for k in keys:
			time = self.average_all(data, key_filter(k), 'time', time_shift)
			nodes = self.average_all(data, key_filter(k), 'nodes', nodes_shift)
			speedups.append(100 * (baseline_time / time - 1))
			nodesreds.append(100 * (1 - nodes / baseline_nodes))

		f = open(output_dir + '/' + 'tab3_iskn_features.txt', 'w')
		f.write("Speed-up (%)" + '\n')
		for i, label in enumerate(labels):
			f.write((label + '\t' + str(round(speedups[i], 2))).expandtabs(40) + '\n')
		f.write('\n')
		f.write("Node reduction (%)" + '\n')
		for i, label in enumerate(labels):
			f.write((label + '\t' + str(round(nodesreds[i], 2))).expandtabs(40) + '\n')
		f.write('\n')
		f.close()


class PlotterIndepSetKnapsackGap(object):
	"""Plotter for Figure 9"""

	instance_type = 'iskn'
	run_directory = 'output_iskn_m'
	required_fields = ['bound', 'bddbound', 'primalbound', 'lpbound', 'lpcutsbound',
		'time', 'firstlptime', 'bddtime', 'lagtime', 'bddlagtime']
	filename_filter = ''
	filename_filter_per_field = {'time': 'noboundscuts', 'firstlptime': 'noboundscuts',
		'lpbound': 'noboundscuts', 'lpcutsbound': 'noboundscuts'}

	def plot(self, data, avg_data, output_dir):
		for inst_type in ['iskn_clique_n200', 'iskn_clique_n1000']:
			self.plot_gap(data, avg_data, output_dir, inst_type)

	def gap(self, dual_bound, primal_bound):
		return 100 * ((dual_bound - primal_bound) / primal_bound)

	def compute_gaps_avg(self, dual_bounds, primal_bounds):
		gaps_avg = {}
		for k in dual_bounds.iterkeys():
			assert k in primal_bounds
			gaps = {instance : self.gap(dual_bounds[k][instance], primal_bounds[k][instance]) for instance in dual_bounds[k]}
			gaps_avg[k] = numpy.mean(gaps.values())
		return gaps_avg

	def plot_gap(self, data, avg_data, output_dir, inst_type):
		keys = ['w1000_ct_ddonly', 'w1000_ct', 'w1000_ctprop', 'lpbound', 'lpcutsbound']
		nkeys = len(keys)
		plot_properties = [
			['Conflict graph (DD only)', colors[0], markers[0], '-'],
			['Conflict graph with Lagrangian', colors[2], markers[2], '-'],
			['Conflict graph with Lagrangian and propagation', colors[1], markers[1], '-'],
			['LP bound', 'black', '.', '--'],
			['LP bound at the end of root', '#888888', '.', '--']]

		selector = key_selectors_iskn['num_knapsack']
		primal_bounds = collect_plot_dict(data, selector, key_filter(inst_type + '.*' + 'full'), 'primalbound', use_avg_data=False)

		gaps_avg_all = []
		dual_bounds = collect_plot_dict(data, selector, key_filter(inst_type + '.*' + 'w1000_ct$'), 'bddbound', use_avg_data=False)
		gaps_avg_all.append(self.compute_gaps_avg(dual_bounds, primal_bounds))
		dual_bounds = collect_plot_dict(data, selector, key_filter(inst_type + '.*' + 'w1000_ct$'), 'bound', use_avg_data=False)
		gaps_avg_all.append(self.compute_gaps_avg(dual_bounds, primal_bounds))
		dual_bounds = collect_plot_dict(data, selector, key_filter(inst_type + '.*' + 'w1000_ctprop$'), 'bound', use_avg_data=False)
		gaps_avg_all.append(self.compute_gaps_avg(dual_bounds, primal_bounds))
		dual_bounds = collect_plot_dict(data, selector, key_filter(inst_type + '.*' + 'noboundscuts$'), 'lpbound', use_avg_data=False)
		gaps_avg_all.append(self.compute_gaps_avg(dual_bounds, primal_bounds))
		dual_bounds = collect_plot_dict(data, selector, key_filter(inst_type + '.*' + 'noboundscuts$'), 'lpcutsbound', use_avg_data=False)
		gaps_avg_all.append(self.compute_gaps_avg(dual_bounds, primal_bounds))

		times_avg_all = []
		times_avg_all.append(collect_plot_dict(avg_data, selector, key_filter(inst_type + '.*' + 'w1000_ct$'), 'bddtime'))
		times_avg_all.append(collect_plot_dict(avg_data, selector, key_filter(inst_type + '.*' + 'w1000_ct$'), 'bddlagtime'))
		times_avg_all.append(collect_plot_dict(avg_data, selector, key_filter(inst_type + '.*' + 'w1000_ctprop$'), 'bddlagtime'))
		times_avg_all.append(collect_plot_dict(avg_data, selector, key_filter(inst_type + '.*' + 'noboundscuts$'), 'firstlptime'))
		times_avg_all.append(collect_plot_dict(avg_data, selector, key_filter(inst_type + '.*' + 'noboundscuts$'), 'time'))

		plt.clf()

		xaxis = sorted(list(gaps_avg_all[0].keys()))
		minx = xaxis[0]
		maxx = xaxis[-1]
		step = xaxis[1] - xaxis[0]
		width = step * 0.15
		dist = width * 1.05
		interval = step

		f, (a0, a1) = plt.subplots(2,1, figsize=(8,7.5), gridspec_kw = {'height_ratios':[3, 1]})
		for i, gaps in enumerate(gaps_avg_all):
			plot_dict(gaps, plot_properties[i][0], color=plot_properties[i][1], marker=plot_properties[i][2],
				ls=plot_properties[i][3], plt=a0)
		a0.set_ylabel('Gap w.r.t. primal bound (%)')
		a0.set_xlim(minx - nkeys * 0.5 * dist, maxx + nkeys * 0.5 * dist)
		a0.legend(loc='upper center', fontsize=12, bbox_to_anchor=(0.5, -0.65))

		for x in xaxis:
			y = [times_avg_all[i][x] for i in range(nkeys)]
			pos = [x + (i - 0.5 * nkeys) * dist + width * 0.5 for i in range(nkeys)]
			a1.bar(pos, y, width, align='center', color=[plot_properties[i][1] for i in range(nkeys)], linewidth=0)

		offset_y = a1.get_ylim()[1] * 0.08
		for i, times_avg in enumerate(times_avg_all):
			plot_dict({k + (i - 0.5 * nkeys) * dist + width * 0.5: v + offset_y for k, v in times_avg.items()},
				plot_properties[i][0], ls='None', color=plot_properties[i][1], marker=plot_properties[i][2], plt=a1)

		a1.set_xlabel('Number of knapsack constraints')
		a1.set_ylabel('Time (s)')
		a1.set_xlim(minx - nkeys * 0.5 * dist, maxx + nkeys * 0.5 * dist)
		a1.set_ylim(a1.get_ylim()[0], a1.get_ylim()[1] + offset_y)
		a1.yaxis.set_major_locator(ticker.MaxNLocator(nbins=5))

		fig_str = 'fig9a' if inst_type == 'iskn_clique_n200' else 'fig9b'
		plt.savefig(output_dir + '/' + fig_str + '_bounds_root_mscale_boundstime_' + inst_type + '.pdf', bbox_inches="tight")


class PlotterIndepSetKnapsackAppendix(object):
	"""Plotter for experiments with independent set random instances in appendix"""

	instance_type = 'iskn'
	run_directory = 'output_iskn_n'
	required_fields = ['time', 'totalbddtime', 'totalboundtime', 'nruns', 'nimproves', 'nprunes', 'nprimal']
	filename_filter = ''
	filename_filter_per_field = None

	def __init__(self):
		pass

	def plot(self, data, avg_data, output_dir):
		self.plot_breakdown(data, avg_data, output_dir)
		self.plot_improves(data, avg_data, output_dir)

	def plot_breakdown(self, data, avg_data, output_dir):
		bddtimefrac = {}
		boundtimefrac = {}
		for k, v in data.iteritems():
			if 'withpropwithpbwithpp' in k:
				size = key_selectors_iskn['instance_size'](k)
				bddtimefrac[size] = 100 * sum(v['totalbddtime'].values()) / sum(v['time'].values())
				boundtimefrac[size] = 100 * sum(v['totalboundtime'].values()) / sum(v['time'].values())

		densities = sorted(bddtimefrac.keys())

		f = open(output_dir + '/' + 'app_tabX_iskn_fraction_time_spent.txt', 'w')
		for size in densities:
			f.write(str(size) + '\t' + str(bddtimefrac[size]) + '\t' + str(boundtimefrac[size]) + '\n')
		f.close()

		plt.clf()
		plot_dict(bddtimefrac, 'DD construction', color=colors[1], marker='o', ls='-')
		plot_dict(boundtimefrac, 'Overall DD bound generation', color=colors[2], marker='.', ls='--')
		plt.xlabel('Instance size')
		plt.ylabel('Fraction of time spent')
		plt.legend(loc=0, fontsize=12)
		plt.xlim(10, 90)
		plt.ylim(0, 100)
		plt.savefig(output_dir + '/app_figX_iskn_fraction_time_spent.pdf')

	def plot_improves(self, data, avg_data, output_dir):
		fkey = 'withpropwithpbwithpp'
		nruns_data = collect_plot_dict(avg_data, key_selectors_iskn['instance_size'], key_filter(fkey), 'nruns')
		nimproves_data = collect_plot_dict(avg_data, key_selectors_iskn['instance_size'], key_filter(fkey), 'nimproves')
		nprunes_data = collect_plot_dict(avg_data, key_selectors_iskn['instance_size'], key_filter(fkey), 'nprunes')
		nprimal_data = collect_plot_dict(avg_data, key_selectors_iskn['instance_size'], key_filter(fkey), 'nprimal')

		num_sizes = sorted(nruns_data.keys())

		f = open(output_dir + '/' + 'app_tabX_iskn_nimproves.txt', 'w')
		for size in num_sizes:
			f.write(str(size) + '\t' + str(nruns_data[size]) + '\t' + str(nimproves_data[size]) + '\t' + str(nprunes_data[size]) 
				+ '\t' + str(nprimal_data[size]) + '\n')
		f.close()

		plt.clf()
		plot_dict(nruns_data, '# times called', color=colors[0], marker=markers[0], ls='-')
		plot_dict(nimproves_data, '# times improved upon LP bound', color=colors[1], marker=markers[1], ls='-')
		plot_dict(nprunes_data, '# times pruned node', color=colors[2], marker=markers[2], ls='-')
		plt.xlabel('Instance size')
		plt.ylabel('Number of times')
		plt.legend(loc=0, fontsize=12)
		plt.xlim(10, 90)
		plt.savefig(output_dir + '/app_figX_iskn_improves.pdf')

