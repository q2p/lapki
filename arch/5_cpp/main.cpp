// g++ main.cpp -O3 -fopenmp -march=native -mtune=native --fast-math -lpthread --pedantic -Wall

#include <iostream>
#include <chrono>
#include <random>
#include <pthread.h>
#include <omp.h>

double total_time = 0;	

using Args = std::pair<int, int>;

int dim;
size_t numt;
double* a;
double* b;
double* c;

void *dangle;

std::vector<Args> sections;

double dgemm_openmp() {
  omp_set_num_threads(numt);
  auto start = std::chrono::high_resolution_clock::now();
  #pragma omp parallel for
  for (int i = 0; i < dim; i++) {
    for (int k = 0; k < dim; k++) {
      for (int j = 0; j < dim; j++) {
        c[i*dim+j] += a[i*dim+k] * b[k*dim+j];
      }
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double>(end - start).count();
}

void* dgemm_pthread(void* argv) {
  auto arg = (Args*) argv;
  for (int i = arg->first; i != arg->second; i++) {
    for (int k = 0; k != dim; k++) {
      for (int j = 0; j != dim; j++) {
        c[i*dim+j] += a[i*dim+k] * b[k*dim+j];
      }
    }
  }
  return nullptr;
}

void* dgemm_transpose(void* argv) {
  auto arg = (Args*) argv;
  for (int i = arg->first; i != arg->second; i++) {
    for (int j = 0; j != i + 1; j++) {
      std::swap(b[i*dim+j], b[j*dim+i]);
    }
  }
  return nullptr;
}

void* dgemm_rowed(void* argv) {
  auto arg = (Args*) argv;
  for (int i = arg->first; i != arg->second; i++) {
    for (int j = 0; j != dim; j++) {
      for (int k = 0; k != dim; k++) {
        c[i*dim+j] += a[i*dim+k] * b[j*dim+k];
      }
    }
  }
  return nullptr;
}

double dgemm_pthread_spawn() {
  std::vector<pthread_t> threads(numt);
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < numt; i++) {
    pthread_create(&threads[i], NULL, dgemm_pthread, &sections[i]);
  }
  for (size_t i = 0; i < numt; i++) {
    pthread_join(threads[i], &dangle);
  }
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double>(end - start).count();
}

double dgemm_transpose_spawn() {
  std::vector<pthread_t> threads(numt);
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < numt; i++) {
    pthread_create(&threads[i], NULL, dgemm_transpose, &sections[i]);
  }
  for (size_t i = 0; i < numt; i++) {
    pthread_join(threads[i], &dangle);
  }
  for (size_t i = 0; i < numt; i++) {
    pthread_create(&threads[i], NULL, dgemm_rowed, &sections[i]);
  }
  for (size_t i = 0; i < numt; i++) {
    pthread_join(threads[i], &dangle);
  }
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double>(end - start).count();
}

double dgemm_single() {
  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < dim; i++) {
    for (int k = 0; k < dim; k++) {
      for (int j = 0; j < dim; j++) {
        c[i*dim+j] += a[i*dim+k] * b[k*dim+j];
      }
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double>(end - start).count();
}

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cout << "Not enough arguments\n";
    return 1;
  }

  auto type = argv[1][0];
  numt = std::strtoul(argv[2], (char**)&dangle, 10);
  dim  = std::strtoul(argv[3], (char**)&dangle, 10);
  
  a = new double[dim*dim];
  b = new double[dim*dim];
  c = new double[dim*dim];

  std::random_device dev;
  std::default_random_engine rng(dev());
  std::uniform_real_distribution<double> dist(0, 1);
  for (int i = 0; i != dim*dim; i++) {
    a[i] = dist(rng);
    b[i] = dist(rng);
    c[i] = 0;
  }

  for (size_t i = 0; i != numt; i++) {
    auto& s = sections.emplace_back(dim * i / numt, dim * (i + 1) / numt);
    if (i + 1 == numt) {
      s.second = dim;
    }
  }

  total_time = 0;
  switch (type) {
    case '1': total_time = dgemm_single();          break;
    case '2': total_time = dgemm_pthread_spawn();   break;
    case '3': total_time = dgemm_transpose_spawn(); break;
    case '4': total_time = dgemm_openmp();          break;
  }
  std::cout << "Time: " << total_time << "\n";

  delete[] a;
  delete[] b;
  delete[] c;

  return 0;
}