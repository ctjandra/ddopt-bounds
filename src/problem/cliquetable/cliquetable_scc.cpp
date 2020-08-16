/** View a conflict graph as an implication graph and run Tarjan's strongly connected components algorithm */

#include "cliquetable_scc.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/strong_components.hpp>

using namespace boost;


void make_domain_consistent(CliqueTableInstance* inst, IntSet* state_intset)
{
	typedef adjacency_list<vecS, vecS, bidirectionalS,
	        property<vertex_color_t, default_color_type, property<vertex_degree_t,int>>> ImplGraph;
	typedef graph_traits<ImplGraph>::vertex_descriptor Vertex;

	ImplGraph graph(2 * inst->nvars);

	// cout << "Clique table:" << endl;
	// inst->print();

	// For every edge (i,j), we have two arcs (i, not j) and (j, not i)
	for (int i = 0; i < inst->nvars; ++i) {
		for (int j = inst->adj[i].get_first(); j != inst->adj[i].get_end(); j = inst->adj[i].get_next(j)) {
			if (i < j && i != inst->get_complement(j)) {
				add_edge(i, inst->get_complement(j), graph);
				add_edge(j, inst->get_complement(i), graph);
			}
		}
	}

	typedef graph_traits<ImplGraph>::edge_iterator edge_iterator;
	pair<edge_iterator, edge_iterator> ei = edges(graph);

	// Debugging info
	// cout << endl << "Implication graph:" << endl;
	// for (edge_iterator it = ei.first; it != ei.second; ++it) {
	// 	cout << "(" << source(*it, graph) << ", " << target(*it, graph) << ")" << " ";
	// }
	// cout << endl;

	// Find strongly connected components
	int nv = num_vertices(graph);
	vector<int> component(nv), discover_time(nv);
	vector<default_color_type> color(nv);
	vector<Vertex> root(nv);
	int num = strong_components(graph, make_iterator_property_map(component.begin(), get(vertex_index, graph)),
	                            root_map(make_iterator_property_map(root.begin(), get(vertex_index, graph))).
	                            color_map(make_iterator_property_map(color.begin(), get(vertex_index, graph))).
	                            discover_time_map(make_iterator_property_map(discover_time.begin(), get(vertex_index, graph))));

	// cout << num << " strong components" << endl;

	// for (int j = num - 1; j >= 0; --j) {
	// 	cout << "[ ";
	// 	for (int i = 0; i < component.size(); ++i) {
	// 		if (j == component[i]) {
	// 			cout << i << " ";
	// 		}
	// 	}
	// 	cout << "] ";
	// }

	// Assert reverse topological ordering
	for (edge_iterator it = ei.first; it != ei.second; ++it) {
		assert(component[source(*it, graph)] >= component[target(*it, graph)]);
	}

	vector<int> topological_order;
	for (int j = num - 1; j >= 0; --j) {
		for (int i = 0; i < (int) component.size(); ++i) {
			if (j == component[i]) {
				topological_order.push_back(i);
			}
		}
	}

	// cout << "Topological order:  ";
	// for (int i : topological_order) {
	// 	cout << i << " ";
	// }
	// cout << endl;

	vector<vector<int>> vertices_in_component(component.size());
	for (int i = 0; i < nv; ++i) {
		vertices_in_component[component[i]].push_back(i);
	}

	// // Debugging info
	// for (int i = 0; i < component.size(); ++i) {
	// 	cout << "Component " << i << ":  ";
	// 	for (int j : vertices_in_component[i]) {
	// 		cout << j << " ";
	// 	}
	// 	cout << endl;
	// }

	// Traverse components in topological order
	vector<set<int>> ancestors(component.size());
	for (int c = nv - 1; c >= 0; --c) {
		for (int v : vertices_in_component[c]) {
			typename graph_traits<ImplGraph>::adjacency_iterator u, u_end;
			for (tie(u, u_end) = adjacent_vertices(v, graph); u != u_end; ++u) {
				// Add current component and its own ancestors to ancestor list of adjacent component
				ancestors[component[*u]].insert(c);
				for (int ancestor : ancestors[c]) {
					ancestors[component[*u]].insert(ancestor);
				}
			}
		}
	}

	// // Debugging info
	// for (int i = 0; i < ancestors.size(); ++i) {
	// 	cout << "Component " << i << " - ancestors:  ";
	// 	for (int j : ancestors[i]) {
	// 		cout << j << " ";
	// 	}
	// 	cout << endl;
	// }

	// If the same component contains both complement nodes, remove them
	for (int v = 0; v < inst->nvars; ++v) {
		if (component[v] == component[inst->get_complement(v)]) {
			state_intset->remove(v);
			state_intset->remove(inst->get_complement(v));
		}
	}

	// For each vertex, check if it precedes its component; if so, remove the vertex from the domain
	for (int v = 0; v < nv; ++v) {
		int complement = component[inst->get_complement(v)];
		if (ancestors[complement].find(component[v]) != ancestors[complement].end()) {
			// There exists a path from v to complement of v
			state_intset->remove(v);
		}
	}
}
