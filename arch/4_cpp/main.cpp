// g++ main.cpp -O3 -march=native -mtune=native --fast-math --pedantic -Wall

#include <iostream>
#include <chrono>
#include <random>

int size;
double* a;
double* b;
double* c;

void dgemm_naive() {
  for (int i = 0; i != size; i++) {
    for (int j = 0; j != size; j++) {
      for (int k = 0; k != size; k++) {
        c[i*size+j] += a[i*size+k] * b[k*size+j];
      }
    }
  }
}

void dgemm_ordered() {
  for (int i = 0; i != size; i++) {
    for (int k = 0; k != size; k++) {
      for (int j = 0; j != size; j++) {
        c[i*size+j] += a[i*size+k] * b[k*size+j];
      }
    }
  }
}

void dgemm_transpose() {
  for (int i = 0; i != size; i++) {
    for (int j = 0; j != i; j++) {
      std::swap(b[i*size+j], b[j*size+i]);
    }
  }
  for (int i = 0; i != size; i++) {
    for (int j = 0; j != size; j++) {
      for (int k = 0; k != size; k++) {
        c[i*size+j] += a[i*size+k] * b[j*size+k];
      }
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout << "Not enough arguments\n";
    return 1;
  }

  char* size_end;
  auto type = argv[1][0];
  size = std::strtoul(argv[2], &size_end, 10);

  a = new double[size*size];
  b = new double[size*size];
  c = new double[size*size];
  std::random_device dev;
  std::default_random_engine rng(dev());
  std::uniform_real_distribution<double> dist(0, 1);
  for (size_t i = 0; i != size*size; i++) {
    a[i] = dist(rng);
    b[i] = dist(rng);
    c[i] = 0;
  }
  auto start = std::chrono::high_resolution_clock::now();
  switch (type) {
    case 'n': dgemm_naive();     break;
    case 'o': dgemm_ordered();   break;
    case 't': dgemm_transpose(); break;
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration<double>(end - start).count();
  std::cout << "Time: " << time << "\n";
  delete[] a;
  delete[] b;
  delete[] c;
  return 0;
}