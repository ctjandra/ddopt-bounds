import random

def generate_random_graph(fname, n, p):
	f = open(fname, 'w')
	f.write('c "random instance"\n')

	edges = []
	for i in range(n):
		for j in range(i+1, n):
			if random.uniform(0, 1) < p:
				edges.append((i,j))

	f.write('p edges ' + str(n) + ' ' + str(len(edges)) + '\n')
	for e in edges:
		f.write('e ' + str(e[0]+1) + ' ' + str(e[1]+1) + '\n')


for n in [150]:
	for p in [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9]:
		for i in range(16):
			fname = 'random_' + str(n) + '_' + str(int(p*100)) + '_' + str(i) + '.clq'
			generate_random_graph(fname, n, p)
