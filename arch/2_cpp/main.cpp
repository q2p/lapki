#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cstring>
#include <thread>
#include <iostream>
#include <fstream>

constexpr uint32_t SAMPLES = 32;
constexpr uint32_t SUB_SAMPLES = 10000;
constexpr size_t RUNNERS = 6;

std::string proc_name() {
  return "AMD Ryzen 3600";
  std::ifstream cpufile("/proc/cpuinfo");
  std::string line;
  while(cpufile) {
    std::getline(cpufile, line);
    if (line.rfind("model name\t:", 0) == 0) {
      line.erase(0,13);
      cpufile.close();
      return line;
    }
  }
}

struct State {
  float fa, fb, ff;
  double da, db, df;
};

void pow_f32(State& state) { state.ff += std::pow(state.fa, state.fb); }
void pow_f64(State& state) { state.df += std::pow(state.da, state.db); }
void sin_f32(State& state) { state.ff += std::sin(state.da); }
void sin_f64(State& state) { state.df += std::sin(state.da); }
void sqrt_f32(State& state) { state.ff += std::sqrt(state.da); }
void sqrt_f64(State& state) { state.df += std::sqrt(state.da); }

struct Runner {
  char* name;
  char* op_type;
  uint8_t offset;
  uint32_t ins_count;
  void (*func)(State&);
  std::thread* handle;
};

double time_history[SAMPLES*RUNNERS];
Runner runners[RUNNERS] = {
  Runner {  "pow_f32", "f32", 0,  9, &pow_f32, nullptr },
  Runner {  "pow_f64", "f64", 1,  9, &pow_f64, nullptr },
  Runner {  "sin_f32", "f32", 2, 12, &sin_f32, nullptr },
  Runner {  "sin_f64", "f64", 3, 12, &sin_f64, nullptr },
  Runner { "sqrt_f32", "f32", 4, 17, &sqrt_f32, nullptr },
  Runner { "sqrt_f64", "f64", 5, 17, &sqrt_f64, nullptr },
};

void perform(Runner* r) {
  srand(time(NULL));
  State s = { 0.0f, 0.0f, 0.0f, 0.0, 0.0, 0.0 };
  
  for(uint32_t i = 0; i != SAMPLES; i++) {
    s.da = 0.1 + ((double) rand() / (double) RAND_MAX);
    s.db = 0.1 + ((double) rand() / (double) RAND_MAX);
    s.fa = 0.1f + ((float) rand() / (float) RAND_MAX);
    s.fb = 0.1f + ((float) rand() / (float) RAND_MAX);
    
    auto start = std::chrono::high_resolution_clock::now();
    for(uint32_t i = 0; i != SUB_SAMPLES; i++) {
      r->func(s);
    }
    auto end = std::chrono::high_resolution_clock::now();
    time_history[r->offset*SAMPLES+i] = std::chrono::duration<double>(end - start).count();
  }

  std::cout << s.ff << s.df << '\n';
}

int main() {
  for (auto &r : runners) {
    r.handle = new std::thread(perform, &r);
  }
  for (auto &r : runners) {
    r.handle->join();
    delete r.handle;
  }
  
  auto cpu = proc_name();

  std::ofstream csv("out.csv");

  csv << "PModel,Task,OpType,Opt,InsCount,Timer,Time,LNum,AvTime,AbsErr,RelErr,TaskPerf\n";
  for (auto &r : runners) {
    double total = 0;
    for (uint32_t i = 0; i != SAMPLES; i++) {
      total += time_history[r.offset * SAMPLES + i];
    }
    double avg = total / (double) (SAMPLES * SUB_SAMPLES);
    for (uint32_t i = 0; i != SAMPLES; i++) {
      double secs = time_history[r.offset * SAMPLES + i] / (double) SUB_SAMPLES;
      double diff = std::fabs(avg - secs);
      double rel_diff = diff / avg;
      double perf = 1 / secs;
      csv << cpu << ',';
      csv << r.name << ',';
      csv << r.op_type << ',';
      csv << "O3,";
      csv << r.ins_count << ',';
      csv << "high_resolution_clock,";
      csv << secs << ',';
      csv << i << ',';
      csv << avg << ',';
      csv << diff << ',';
      csv << rel_diff << ',';
      csv << perf << '\n';
    }
  }
  
  csv.flush();
  csv.close();
  
  return 0;
}