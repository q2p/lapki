import math
import networkx as nx
import matplotlib.pyplot as plt
import random
import os

vert = 0
V = []
edges = []

for f in os.listdir('.'):
  if f.endswith('.png'):
    os.remove(f) 

def add_edge(i, j, cap):
  global V
  global edges
  
  if i != j and V[i][j] == 0 and V[j][i] == 0:
    V[i][j] = cap
    edges.append([i, j])

def rand_v(size, edges_per_vertex):
  global V
  global vert
  global edges
  
  vert = size
  V = [[0 for j in range(vert)] for i in range(vert)]

  for _ in range(vert*edges_per_vertex):
    i = random.randint(0,vert-2)
    j = random.randint(i+1, vert-1)
    add_edge(i, j, random.randint(1,20))
  
  for i in range(vert):
    inputs = 0
    outputs = 0
    for j in range(vert):
      if V[i][j] != 0: outputs += 1
      if V[j][i] != 0: inputs += 1
    if outputs != 0 and inputs == 0:
      add_edge(0, i, random.randint(1,20))
    if inputs != 0 and outputs == 0:
      add_edge(i, vert-1, random.randint(1,20))

def file_v(name):
  global V
  global vert
  global edges

  file = open(name, 'r')
  vert = int(file.readline().strip())
  V = [[0 for j in range(vert)] for i in range(vert)]
  for line in file:
    x = line[:len(line)-1].split()
    i = int(x[0]) # row
    j = int(x[1]) # column
    cap = int(x[2]) # weight
    V[i][j] = cap
    V[j][i] = 0
    edges.append([i,j])

  file.close()

file_v('edges1.txt')
# file_v('edges2.txt')
# rand_v(6, 2)

imnum = 0
def print_graph():
  global imnum
  global V
  global edges
  G = nx.DiGraph(directed=True)

  for i in range(vert):
    G.add_node(str(i))
  for x in edges:
    G.add_edges_from([(str(x[0]), str(x[1]), {'capacity': str(V[x[0]][x[1]])+'/'+str(V[x[0]][x[1]]+V[x[1]][x[0]])})])

  pos = nx.spring_layout(G, seed=18, iterations=500)
  nx.draw_networkx_nodes(G, pos, node_color='lightblue', node_size=500)
  nx.draw_networkx_edges(G, pos, edge_color='grey')
  nx.draw_networkx_labels(G, pos, font_size=12, font_family='sans-serif')
  nx.draw_networkx_edge_labels(
    G, pos, edge_labels={(u, v): d['capacity'] for u, v, d in G.edges(data=True)}
  )
  plt.axis('off')
  plt.savefig(f'{imnum}.png')
  imnum += 1

def find_flows(V):
  global vert

  source_id = 0
  sink_id = vert-1

  max_flows = 0
  while True:
    print_graph()
    current_id = source_id
    path = [(math.inf, -1, source_id)]
    visited = {source_id}

    while current_id != sink_id:
      max_cap = 0
      next_id = -1
      for j in range(vert):
        if j in visited:
          continue;
        if max_cap < V[current_id][j]:
          max_cap = V[current_id][j]
          next_id = j

      if next_id == -1:
        if current_id == source_id:
          return max_flows
        current_id = path.pop()[1]
      else:
        path.append((max_cap, current_id, next_id))
        visited.add(next_id)
        current_id = next_id
    
    flow = min([x[0] for x in path])
    max_flows += flow
    print(f'Поток {flow} через путь : {[x[2] for x in path]}')

    for p in path:
      i, j = p[1], p[2]
      if i == -1:
        continue
      V[i][j] -= flow
      V[j][i] += flow

print(f'Максимальный поток равен: {find_flows(V)}')
