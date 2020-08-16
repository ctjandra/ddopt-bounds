import collections
import numpy
import re
import itertools
import matplotlib.pyplot as plt

from settings import *


# Warning: In the paper, we manually clean up plots for cases where time limit was hit.
# This is controlled by the argument manual_timelimit_cleanup, which is False by default.
# Experiments "hidden" by manual cleanups were performed, but may be omitted in this package.
class PlotterIndepSet(object):
	"""Plotter for Figures 5 to 8: experiments with independent set random instances"""

	instance_type = 'indepset'
	required_fields = ['time', 'nodes']
	filename_filter = ''
	filename_filter_per_field = None

	def __init__(self, inst_type, manual_timelimit_cleanup=False):
		assert inst_type in ['random_150', 'random_300']
		self.inst_type = inst_type
		self.run_directory = 'output_is_n' + self.inst_type[-3:]
		self.manual_timelimit_cleanup = manual_timelimit_cleanup
		if inst_type == 'random_150':
			self.nvars_plot_keys = ['v50_nrppp', 'v100_nrppp', 'v150_nrppp']
			self.nvars_plot_colors = {'v50_nrppp': colors[0], 'v100_nrppp': colors[1], 'v150_nrppp': colors[2]}
			self.nvars_plot_markers = {'v50_nrppp': markers[0], 'v100_nrppp': markers[1], 'v150_nrppp': markers[2]}
			self.nvars_plot_labels = {'v50_nrppp': 'Subproblem size threshold of 50', 'v100_nrppp': 'Subproblem size threshold of 100', 'v150_nrppp': 'Subproblem size threshold of 150'}
		else:
			self.nvars_plot_keys = ['v100_nrppp', 'v200_nrppp', 'v300_nrppp']
			self.nvars_plot_colors = {'v100_nrppp': colors[0], 'v200_nrppp': colors[1], 'v300_nrppp': colors[2]}
			self.nvars_plot_markers = {'v100_nrppp': markers[0], 'v200_nrppp': markers[1], 'v300_nrppp': markers[2]}
			self.nvars_plot_labels = {'v100_nrppp': 'Subproblem size threshold of 100', 'v200_nrppp': 'Subproblem size threshold of 200', 'v300_nrppp': 'Subproblem size threshold of 300'}

	def plot(self, data, avg_data, output_dir):
		self.plot_times_overall(data, avg_data, output_dir)
		self.plot_nodes_overall(data, avg_data, output_dir)
		if self.inst_type == 'random_150':  # random_300 not in paper
			self.plot_times_nvars(data, avg_data, output_dir)
			self.plot_nodes_nvars(data, avg_data, output_dir)
		self.plot_speedup(data, avg_data, output_dir)

	def plot_times_overall(self, data, avg_data, output_dir):
		fkey = 'v100_nrppp' if self.inst_type == 'random_150' else 'v200_nrppp'
		times_mip_dd = collect_plot_dict(avg_data, key_selectors_indepset['density'], key_filter(fkey), 'time')
		times_mip_only = collect_plot_dict(avg_data, key_selectors_indepset['density'], key_filter('nobounds'), 'time')

		# Warning: Manually fix cases where time limit was hit frequently.
		if self.manual_timelimit_cleanup and self.inst_type == 'random_300':
			for d in [10, 20, 30]:
				times_mip_dd[d] = 3600
				times_mip_only[d] = 3600

		print('times_mip_dd', times_mip_dd)
		print('times_mip_only', times_mip_only)

		plt.clf()
		if self.inst_type == 'random_300':
			plt.plot([10, 90], [3600, 3600], color='#888888', ls=':')
		plot_dict(times_mip_dd, 'MIP + DD bounds', color=colors[1], marker='o', ls='-')
		plot_dict(times_mip_only, 'MIP', color='black', marker='.', ls='--')
		plt.xlabel('Density')
		plt.ylabel('Total solving time (s)')
		plt.legend(loc=0, fontsize=12)
		fig_str = 'fig5a' if self.inst_type == 'random_150' else 'fig6a'
		plt.savefig(output_dir + '/' + fig_str + '_bounds_pure_n_times_overall_' + self.inst_type + '.pdf')

	def plot_nodes_overall(self, data, avg_data, output_dir):
		fkey = 'v100_nrppp' if self.inst_type == 'random_150' else 'v200_nrppp'
		nodes_mip_dd = collect_plot_dict(avg_data, key_selectors_indepset['density'], key_filter(fkey), 'nodes')
		nodes_mip_only = collect_plot_dict(avg_data, key_selectors_indepset['density'], key_filter('nobounds'), 'nodes')

		# Warning: Manually remove cases where time limit was hit frequently.
		if self.manual_timelimit_cleanup and self.inst_type == 'random_300':
			nodes_mip_dd = {k : v for k, v in nodes_mip_dd.iteritems() if k >= 40}
			nodes_mip_only = {k : v for k, v in nodes_mip_only.iteritems() if k >= 60}

		print('nodes_mip_dd', nodes_mip_dd)
		print('nodes_mip_only', nodes_mip_only)

		plt.clf()
		plot_dict(nodes_mip_dd, 'MIP + DD bounds', color=colors[1], marker='o', ls='-')
		plot_dict(nodes_mip_only, 'MIP', color='black', marker='.', ls='--')
		plt.xlabel('Density')
		plt.ylabel('Number of nodes')
		plt.legend(loc=0, fontsize=12)
		plt.yscale('log')
		plt.xlim(10, 90)
		fig_str = 'fig5b' if self.inst_type == 'random_150' else 'fig6b'
		plt.savefig(output_dir + '/' + fig_str + '_bounds_pure_n_nodes_overall_' + self.inst_type + '.pdf')
	
	def plot_times_nvars(self, data, avg_data, output_dir):
		times_nvars_mip_only = collect_plot_dict(avg_data, key_selectors_indepset['density'], key_filter('nobounds'), 'time')

		plt.clf()
		for k in self.nvars_plot_keys:
			times_nvars_mip_dd = collect_plot_dict(avg_data, key_selectors_indepset['density'], key_filter(k), 'time')
			plot_dict(times_nvars_mip_dd, self.nvars_plot_labels[k], color=self.nvars_plot_colors[k], 
				marker=self.nvars_plot_markers[k], ls='-')
		plot_dict(times_nvars_mip_only, 'MIP', color='black', marker='.', ls='--')
		plt.xlabel('Density')
		plt.ylabel('Total solving time (s)')
		plt.legend(loc=0, fontsize=12)
		if self.inst_type == 'random_300':
			plt.ylim(0, 2000)
		plt.savefig(output_dir + '/' + 'fig7a_bounds_pure_n_times_nvars_' + self.inst_type + '.pdf')

	def plot_nodes_nvars(self, data, avg_data, output_dir):
		nodes_nvars_mip_only = collect_plot_dict(avg_data, key_selectors_indepset['density'], key_filter('nobounds'), 'nodes')

		plt.clf()
		for k in self.nvars_plot_keys:
			nodes_nvars_mip_dd = collect_plot_dict(avg_data, key_selectors_indepset['density'], key_filter(k), 'nodes')
			plot_dict(nodes_nvars_mip_dd, self.nvars_plot_labels[k], color=self.nvars_plot_colors[k], 
				marker=self.nvars_plot_markers[k], ls='-')
		plot_dict(nodes_nvars_mip_only, 'MIP', color='black', marker='.', ls='--')
		plt.xlabel('Density')
		plt.ylabel('Number of nodes')
		plt.legend(loc=0, fontsize=12)
		plt.yscale('log')
		plt.savefig(output_dir + '/' + 'fig7b_bounds_pure_n_nodes_nvars_' + self.inst_type + '.pdf')

	def plot_speedup(self, data, avg_data, output_dir):
		if self.inst_type == 'random_150':
			speedup_plot_colors = {'v100_nrponly': colors[0], 'v100_pponly': colors[2], 'v100_simple': colors[3]}
			speedup_plot_markers = {'v100_nrponly': markers[0], 'v100_pponly': markers[2], 'v100_simple': markers[3]}
			speedup_plot_labels = {
				'v100_nrppp': 'Baseline: Both primal pruning and heuristic enabled',
				'v100_nrponly': 'Primal pruning disabled',
				'v100_pponly': 'Primal heuristic disabled',
				'v100_simple': 'Primal pruning and heuristic disabled'}
			baseline_key = 'v100_nrppp'
		else:
			speedup_plot_colors = {'v200_nrponly': colors[0], 'v200_pponly': colors[2], 'v200_simple': colors[3]}
			speedup_plot_markers = {'v200_nrponly': markers[0], 'v200_pponly': markers[2], 'v200_simple': markers[3]}
			speedup_plot_labels = {
				'v200_nrppp': 'Baseline: Both primal pruning and heuristic enabled',
				'v200_nrponly': 'Primal pruning disabled',
				'v200_pponly': 'Primal heuristic disabled',
				'v200_simple': 'Primal pruning \nand heuristic disabled'}
			baseline_key = 'v200_nrppp'
		speedup_plot_keys = sorted(list(speedup_plot_labels.keys()))
		speedup_plot_keys.remove(baseline_key)

		baseline_time_plot = collect_plot_dict(avg_data, key_selectors_indepset['density'], key_filter(baseline_key), 'nodes')
		speedup_plots = {}
		for k in speedup_plot_keys:
			time_plot = collect_plot_dict(avg_data, key_selectors_indepset['density'], key_filter(k), 'nodes')
			speedup_plots[k] = {x : (time_plot[x] / baseline_time_plot[x]) for x in time_plot.keys()}

		# Warning: Manually remove cases where time limit was hit frequently.
		if self.manual_timelimit_cleanup and self.inst_type == 'random_300':
			speedup_plots = {k : v for k, v in speedup_plots.iteritems() if k >= 40}

		plt.clf()
		for k in speedup_plot_keys:
			plot_dict(speedup_plots[k], speedup_plot_labels[k], color=speedup_plot_colors[k], 
				marker=speedup_plot_markers[k], ls='-')
		plt.plot([10, 90], [1, 1], color='#888888', ls=':')
		plt.xlabel('Density')
		plt.ylabel('Solving time ratio')
		if self.inst_type == 'random_150':
			plt.legend(loc=0, fontsize=12)
		else:
			plt.legend(loc=2, fontsize=12)
		plt.xlim(10, 90)
		plt.ylim(0.9, 2.5)
		fig_str = 'fig8a' if self.inst_type == 'random_150' else 'fig8b'
		plt.savefig(output_dir + '/' + fig_str + '_bounds_pure_n_times_featureratio_' + self.inst_type + '.pdf')


class PlotterIndepSetAppendix(object):
	"""Plotter for Figures in appendix: experiments with independent set random instances"""

	instance_type = 'indepset'
	required_fields = ['time', 'totalbddtime', 'totalboundtime', 'nruns', 'nimproves', 'nprunes']
	filename_filter = ''
	filename_filter_per_field = None

	def __init__(self, inst_type):
		assert inst_type in ['random_150', 'random_300']
		self.inst_type = inst_type
		self.run_directory = 'output_is_n' + self.inst_type[-3:]

	def plot(self, data, avg_data, output_dir):
		self.plot_breakdown(data, avg_data, output_dir)
		self.plot_improves(data, avg_data, output_dir)

	def plot_breakdown(self, data, avg_data, output_dir):
		fkey = 'v100' if self.inst_type == 'random_150' else 'v200'
		bddtimefrac = {}
		boundtimefrac = {}
		for k, v in data.iteritems():
			if 'nrppp' in k and fkey in k:
				density = key_selectors_indepset['density'](k)
				bddtimefrac[density] = 100 * sum(v['totalbddtime'].values()) / sum(v['time'].values())
				boundtimefrac[density] = 100 * sum(v['totalboundtime'].values()) / sum(v['time'].values())

		densities = sorted(bddtimefrac.keys())

		f = open(output_dir + '/' + 'app_tab_fraction_time_spent_' + self.inst_type + '.txt', 'w')
		for density in densities:
			f.write(str(density) + '\t' + str(bddtimefrac[density]) + '\t' + str(boundtimefrac[density]) + '\n')
		f.close()

		plt.clf()
		plot_dict(bddtimefrac, 'DD construction', color=colors[1], marker='o', ls='-')
		plot_dict(boundtimefrac, 'Overall DD bound generation', color=colors[2], marker='.', ls='--')
		plt.xlabel('Density')
		plt.ylabel('Fraction of time spent')
		plt.legend(loc=0, fontsize=12)
		plt.xlim(10, 90)
		plt.ylim(0, 100)
		plt.savefig(output_dir + '/app_fig_fraction_time_spent_' + self.inst_type + '.pdf')

	def plot_improves(self, data, avg_data, output_dir):
		fkey = 'v100_nrppp' if self.inst_type == 'random_150' else 'v200_nrppp'
		nruns_data = collect_plot_dict(avg_data, key_selectors_indepset['density'], key_filter(fkey), 'nruns')
		nimproves_data = collect_plot_dict(avg_data, key_selectors_indepset['density'], key_filter(fkey), 'nimproves')
		nprunes_data = collect_plot_dict(avg_data, key_selectors_indepset['density'], key_filter(fkey), 'nprunes')

		densities = sorted(nruns_data.keys())

		f = open(output_dir + '/' + 'app_tab_nimproves_' + self.inst_type + '.txt', 'w')
		for density in densities:
			f.write(str(density) + '\t' + str(nruns_data[density]) + '\t' + str(nimproves_data[density]) + '\t' + str(nprunes_data[density]) + '\n')
		f.close()

		plt.clf()
		plot_dict(nruns_data, '# times called', color=colors[0], marker=markers[0], ls='-')
		plot_dict(nimproves_data, '# times improved upon LP bound', color=colors[1], marker=markers[1], ls='-')
		plot_dict(nprunes_data, '# times pruned node', color=colors[2], marker=markers[2], ls='-')
		plt.xlabel('Density')
		plt.ylabel('Number of times')
		plt.legend(loc=0, fontsize=12)
		plt.xlim(10, 90)
		plt.savefig(output_dir + '/app_fig_improves_' + self.inst_type + '.pdf')


class PlotterIndepSetDimacs(object):
	"""Plotter for Table 1: experiments with independent set DIMACS instances"""

	instance_type = 'indepset_dimacs'
	run_directory = 'output_is_dimacs'
	required_fields = ['time', 'nodes']
	filename_filter = ''
	filename_filter_per_field = None

	def plot(self, data, avg_data, output_dir):
		keys = ['nobounds', 'v0.75']
		times = collections.defaultdict(dict)
		nodes = collections.defaultdict(dict)
		for k, v in avg_data.iteritems():
			instance_key = k[0]
			instance = instance_key[:instance_key.find('_complement')]
			for key in keys:
				if key in instance_key:
					times[instance][key] = v['time'][0]
					nodes[instance][key] = v['nodes'][0]
		instances = sorted([instance for instance in times if 'v0.75' in times[instance]])

		f = open(output_dir + '/' + 'tab1_is_dimacs.txt', 'w')
		f.write("Solving time (s)" + '\n')
		f.write("Instance" + '\t' + 'MIP'.expandtabs(40) + '\t' + 'MIP + DD'.expandtabs(40) + '\n')
		for instance in instances:
			f.write(instance 
				+ '\t' + str(round(times[instance]['nobounds'], 2)).expandtabs(40)
				+ '\t' + str(round(times[instance]['v0.75'], 2)).expandtabs(40)
				+ '\n')
		f.write('\n')
		f.write("Number of nodes" + '\n')
		f.write("Instance" + '\t' + 'MIP'.expandtabs(40) + '\t' + 'MIP + DD'.expandtabs(40) + '\n')
		for instance in instances:
			f.write(instance 
				+ '\t' + str(round(nodes[instance]['nobounds'], 2)).expandtabs(40)
				+ '\t' + str(round(nodes[instance]['v0.75'], 2)).expandtabs(40)
				+ '\n')
		f.write('\n')
		f.close()
