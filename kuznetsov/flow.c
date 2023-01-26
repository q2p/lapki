#include <stddef.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef int flow_t;

int seed = 0;
size_t len = -1;
flow_t* V = NULL;
size_t* P = NULL;
size_t* FORWARD = NULL;
char* VISITED = NULL;
size_t* QUEUE = NULL;
int* LEVEL = NULL;

enum {
  ALG_CNT = 3,
  SANITY_TEST = 1024,
  SAMPLES = 16,
  MAX_VERTS = 512,
  NOT = SIZE_MAX,
};

flow_t* at(size_t i, size_t j) {
  return V + (i * len + j);
}

void add_edge(size_t i, size_t j, flow_t cap) {
  if (i != j && *at(i,j) == 0 && *at(j,i) == 0) {
    *at(i,j) = cap;
  }
}

int rng(int a, int b) {
  return a + rand() % (b-a);
}

void alloc_lists(size_t new) {
  if (new != len) {
    len = new;
    if (V) {
      V = (flow_t*) realloc(V, len*len*sizeof(flow_t));
      P = (size_t*) realloc(P, len*sizeof(size_t));
      FORWARD = (size_t*) realloc(FORWARD, len*sizeof(size_t));
      VISITED = (char*) realloc(VISITED, len*sizeof(char));
      QUEUE = (size_t*) realloc(QUEUE, len*sizeof(size_t));
      LEVEL = (int*) realloc(LEVEL, len*sizeof(int));
    } else {
      V = (flow_t*) malloc(len*len*sizeof(flow_t));
      P = (size_t*) malloc(len*sizeof(size_t));
      FORWARD = (size_t*) malloc(len*sizeof(size_t));
      VISITED = (char*) malloc(len*sizeof(char));
      QUEUE = (size_t*) malloc(len*sizeof(size_t));
      LEVEL = (int*) malloc(len*sizeof(int));
    }
  }
  for (size_t i = 0; i != len*len; i++) {
    V[i] = 0;
  }
  for (size_t i = 0; i != len; i++) {
    VISITED[i] = 0;
    P[i] = NOT;
    FORWARD[i] = NOT;
    LEVEL[i] = 0;
  }
}

flow_t rand_flow() {
  return rng((len / 2) | 1, len * 4);
}

void rand_v(size_t verts, size_t edges_per_vertex) {
  alloc_lists(verts);

  srand(seed);

  for (size_t _ = 0; _ != len*edges_per_vertex; _++) {
    size_t j = rng(1, len);
    size_t i = rng(0, j);
    add_edge(i, j, rand_flow());
  }
  
  for (size_t i = 0; i != len; i++) {
    size_t ins = 0;
    size_t ous = 0;
    for (size_t j = 0; j != len; j++) {
      if (*at(i,j) != 0) ous += 1;
      if (*at(j,i) != 0) ins += 1;
    }
    if (ous != 0 && ins == 0) {
      add_edge(0, i, rand_flow());
    }
    if (ins != 0 && ous == 0) {
      add_edge(i, len-1, rand_flow());
    }
  }
}

void file_v(char* name) {
  FILE* file = fopen(name, "r");
  int l;
  fscanf(file, "%d", &l);
  alloc_lists(l);
  while (1) {
    int i, j, cap;
    if (fscanf(file, "%d%d%d", &i, &j, &cap) != 3) {
      break;
    }
    *at(i,j) = cap;
    *at(j,i) = 0;
  }
  fclose(file);
}

void print_edges() {
  for (size_t i = 0; i != len; i++) {
    size_t from = P[i];
    if (from != NOT) {
      printf("p %d -> %d\n", from, i);
    }
  }
  for (size_t i = 0; i != len; i++) {
    size_t to = FORWARD[i];
    if (to != NOT) {
      printf("f %d -> %d\n", i, to);
    }
  }
  for (size_t i = 0; i != len; i++) {
    for (size_t j = 0; j != len; j++) {
      if (*at(i,j) != 0) {
        printf("c %d -> %c%d = %d\n", i, VISITED[j] ? '*' : ' ', j, *at(i,j));
      }
    }
  }
}

void print_path(flow_t flow) {
  printf("Поток: %d, Путь: [ ", flow);
  for (size_t i = len-1; i != -1; i = P[i]) {
    printf("%d <- ", i);
  }
  printf("]\n");
}

flow_t ford_fulkerson_naive() {
  size_t source_id = 0;
  size_t sink_id = len-1;

  flow_t max_flows = 0;

  P[source_id] = NOT;
  VISITED[source_id] = 1;
  while (1) {
    size_t current_id = source_id;
    memset(VISITED+1, 0, sizeof(char)*(len-1));

    while (current_id != sink_id) {
      size_t next = 0;
      while (
        next != len &&
        (
          VISITED[next] ||
          *at(current_id, next) == 0
        )
      ) {
        next++;
      }

      if (next == len) {
        if (current_id == source_id) {
          return max_flows;
        }
        size_t* prev = &P[current_id];
        current_id = *prev;
        *prev = NOT;
      } else {
        P[next] = current_id;
        VISITED[next] = 1;
        current_id = next;
      }
    }

    flow_t flow = INT_MAX;
    size_t to = sink_id;
    while(1) {
      size_t from = P[to];
      if (from == NOT) {
        break;
      }
      flow_t nflow = *at(from, to);
      if (nflow < flow) {
        flow = nflow;
      }
      to = from;
    };
    
    to = sink_id;
    while(1) {
      size_t from = P[to];
      if (from == NOT) {
        break;
      }
      P[to] = NOT;
      *at(from, to) -= flow;
      *at(to, from) += flow;
      to = from;
    };

    max_flows += flow;
  }
}

flow_t ford_fulkerson_memory() {
  size_t source_id = 0;
  size_t sink_id = len-1;

  flow_t max_flows = 0;

  size_t current_id = source_id;
  P[source_id] = NOT;
  VISITED[source_id] = 1;
  memset(VISITED+1, 0, sizeof(char)*(len-1));
  while (1) {
    while (current_id != sink_id) {
      size_t next = FORWARD[current_id];
      if (VISITED[next]) {
        next = NOT;
      }
      if (next == NOT) {
        for (size_t j = 0; j != len; j++) {
          if (!VISITED[j] && *at(current_id,j)) {
            next = j;
            break;
          }
        }
      }

      if (next == NOT) {
        if (current_id == source_id) {
          return max_flows;
        }
        size_t* prev = &P[current_id];
        current_id = *prev;
        *prev = NOT;
        FORWARD[current_id] = NOT;
      } else {
        FORWARD[current_id] = next;
        P[next] = current_id;
        VISITED[next] = 1;
        current_id = next;
      }
    }

    flow_t flow = INT_MAX;
    size_t to = sink_id;
    while(to != NOT) {
      size_t from = P[to];
      if (from == NOT) {
        break;
      }
      flow_t nflow = *at(from, to);
      if (nflow <= flow) {
        flow = nflow;
        current_id = from;
      }
      to = from;
    };

    memset(VISITED+1, 0, sizeof(char)*(len-1));

    to = sink_id;
    char set_vis = 0;
    while(1) {
      size_t from = P[to];
      if (from == NOT) {
        break;
      }
      if (set_vis) {
        VISITED[to] = 1;
      } else if (to == current_id) {
        VISITED[to] = 1;
        set_vis = 1;
      } else {
        P[to] = NOT;
      }

      flow_t* ft = at(from, to);
      flow_t* tf = at(to, from);
      *ft -= flow;
      *tf += flow;
      if (*ft == 0) FORWARD[from] = NOT;
      if (*tf == 0) FORWARD[to]   = NOT;
      to = from;
    }

    max_flows += flow;
  }
}

flow_t dinic() {
  size_t source_id = 0;
  size_t sink_id = len-1;

  flow_t max_flows = 0;
  
  // do_bfs
  while (1) {
    memset(LEVEL, 0, len*sizeof(int));
    LEVEL[source_id] = 1;

    QUEUE[0] = source_id;
    size_t qb = 0;
    size_t qe = 1;
    
    while (qb != qe) {
      size_t i = QUEUE[qb++];
      for (size_t j = 0; j != len; j++) {
        if (*at(i,j) && LEVEL[j] == 0) {
          LEVEL[j] = LEVEL[i] + 1;
          QUEUE[qe++] = j;
        }
      }
    }

    if (LEVEL[sink_id] == 0) {
      return max_flows;
    }

    while (1) {
      size_t current = source_id;
      while (current != sink_id) {
        size_t next = 0;
        while (
          next != len &&
          (
            LEVEL[next] != LEVEL[current] + 1 ||
            *at(current, next) == 0
          )
        ) {
          next++;
        }

        if (next == len) {
          if (current == source_id) {
            goto do_bfs;
          }
          LEVEL[current] = 0;
          size_t* prev = &P[current];
          current = *prev;
          *prev = NOT;
        } else {
          P[next] = current;
          current = next;
        }
      }

      flow_t flow = INT_MAX;
      size_t to = sink_id;
      while(1) {
        size_t from = P[to];
        if (from == NOT) {
          break;
        }
        flow_t nflow = *at(from, to);
        if (nflow < flow) {
          flow = nflow;
        }
        to = from;
      }
      
      to = sink_id;
      while(1) {
        size_t from = P[to];
        if (from == NOT) {
          break;
        }
        P[to] = NOT;
        *at(from, to) -= flow;
        *at(to, from) += flow;
        to = from;
      };

      max_flows += flow;
    }
    do_bfs:
  }

  return max_flows;
}

typedef struct {
  char* name;
  flow_t (*func)();
} Algorithm;

const Algorithm ALGORITHMS[ALG_CNT] = {
  { "Ford-Fulkerson-Naive", &ford_fulkerson_naive },
  { "Ford-Fulkerson-Memory", &ford_fulkerson_memory },
  { "Dinic", &dinic },
};

int main() {
  srand(time(NULL));

  for (int a = 0; a != ALG_CNT; a++) {
    file_v("edges1.txt");
    printf("Максимальный поток: %d, по %s\n", (ALGORITHMS[a].func)(), ALGORITHMS[a].name);
  }

  for (int i = 0; i != SANITY_TEST; i++) {
    flow_t exp = -1;
    seed = rand();
    for (int a = 0; a != ALG_CNT; a++) {
      rand_v(64, 16);
      flow_t r = (ALGORITHMS[a].func)();
      if (exp != -1 && r != exp) {
        puts("Error!");
        return 1;
      }
      exp = r;
    }
  }

  flow_t black_box = 0;
  FILE* file = fopen("algs.csv", "w");
  fprintf(file, "Verticies");
  for (size_t vs = 8; vs < MAX_VERTS; vs = vs + vs / 2) {
    fprintf(file, ",%d", vs);
  }
  fprintf(file, "\n");
  for (int a = 0; a != ALG_CNT; a++) {
    const Algorithm* alg = &ALGORITHMS[a];
    printf("Замеряем %s\n", alg->name);
    fprintf(file, "%s", alg->name);
    for (size_t vs = 8; vs < MAX_VERTS; vs = vs + vs / 2) {
      clock_t start = clock();
      printf("Вершин: %d\n", vs);
      for (size_t s = 0; s != SAMPLES; s++) {
        rand_v(vs, vs/4);
        black_box ^= (alg->func)();
      }
      clock_t end = clock();
      double time_taken = ((double) end - start) / ((double) CLOCKS_PER_SEC);
      fprintf(file, ",%lf", time_taken);
    }
    fprintf(file, "\n");
  }
  fflush(file);
  fclose(file);

  printf("Black Box: %d\n", black_box);

  free(V);
  free(P);
  free(FORWARD);
  free(VISITED);
  free(QUEUE);
  free(LEVEL);
}
