#include <cstdio>
#include <iostream>
#include <fstream>
#include <cmath>

// Новосибирск
// constexpr double exposed_0 = 99;
// constexpr double recovered_0 = 24;
// constexpr double rate_e = 0.999; // ae
// constexpr double rate_i = 0.999; // ai
// constexpr double e_to_i_rate = 0.042; // k
// constexpr double e_recovery_rate = 0.952; // p
// constexpr double i_recovery_rate = 0.999; // beta
// constexpr double death_rate = 0.0188; // mu

// Новосибирск
// constexpr double exposed_0 = 93;
// constexpr double recovered_0 = 96;
// constexpr double rate_e = 0.271; // ae
// constexpr double rate_i = 0.999; // ai
// constexpr double e_to_i_rate = 1.9e-5; // k
// constexpr double e_recovery_rate = 1e-9; // p
// constexpr double i_recovery_rate = 0.007; // beta
// constexpr double death_rate = 0.0009; // mu

// Москва
constexpr double exposed_0 = 783;
constexpr double recovered_0 = 187;
constexpr double rate_e = 0.510; // ae
constexpr double rate_i = 0.618; // ai
constexpr double e_to_i_rate = 0.002; // k
constexpr double e_recovery_rate = 0.187; // p
constexpr double i_recovery_rate = 0.230; // beta
constexpr double death_rate = 0.0052; // mu

constexpr double total_population = 2798170.0;
constexpr double c_isol = 0.0;
constexpr double gamma = 0.0;
constexpr double tau = 2.0;
constexpr double step = 0.1;

struct State {
  double t;
  double susceptible;
  double exposed;
  double infected;
  double recovered;
  double dead;
};

void diff(
  State& prev_s, State& next_s,
  double& prev_v, double& next_v,
  double (*func)(const State& state)
) {
	next_v = prev_v + step     *  func(prev_s)                ;
	next_v = prev_v + step / 2 * (func(prev_s) + func(next_s));
}

constexpr double func_c(const double t) {
	auto res = 1.0 + c_isol * (1.0 - 0.4 * (t - tau));
	if (res < 0.0){
		return 0.0;
	}
	return res;
}

double f_susceptible(const State& state) {
	return -func_c(state.t) * ((rate_i * state.infected  + rate_e * state.exposed) * state.susceptible / total_population) + gamma * state.recovered;
}

double f_exposed(const State& state) {
	return func_c(state.t) * ((rate_i * state.infected + rate_e * state.exposed) * state.susceptible / total_population) - (e_to_i_rate + e_recovery_rate) * state.exposed;
}

double f_infected(const State& state) {
	return e_to_i_rate * state.exposed - i_recovery_rate * state.infected - death_rate * state.infected;
}

double f_recovered(const State& state) {
	return i_recovery_rate * state.infected + e_recovery_rate * state.exposed - gamma * state.recovered;
}

double f_dead(const State& state) {
	return death_rate * state.infected;
}

int main() {
  std::ofstream file("result.txt", std::ofstream::out);

  State states[2];
  states[0].t = 0.0;
	states[0].susceptible = total_population - exposed_0 - recovered_0;
	states[0].exposed = exposed_0;
	states[0].infected = 0;
	states[0].recovered = recovered_0;
	states[0].dead = 0;
	
  size_t prev = 1;
  size_t next = 0;
	while(1) {
    next ^= 1;
    prev ^= 1;

    State& prev_s = states[prev];
    State& next_s = states[next];

    file << prev_s.t << ' ' << prev_s.susceptible << ' ' << prev_s.exposed << ' ';
    file << prev_s.infected << ' ' << prev_s.recovered << ' ' << prev_s.dead << '\n';

    next_s.t = prev_s.t + step;
    if (next_s.t > 100.0) {
      break;
    }

    diff(prev_s, next_s, prev_s.susceptible, next_s.susceptible, &f_susceptible);
    diff(prev_s, next_s, prev_s.exposed, next_s.exposed, &f_exposed);
    diff(prev_s, next_s, prev_s.infected, next_s.infected, &f_infected);
    diff(prev_s, next_s, prev_s.recovered, next_s.recovered, &f_recovered);
    diff(prev_s, next_s, prev_s.dead, next_s.dead, &f_dead);
	}

	file.flush();
  file.close();

  return 0;
}
