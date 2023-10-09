#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

uint64_t pow_mod(uint64_t val, uint64_t exp, uint64_t mod) {
  val %= mod;
  uint64_t ret = 1;
  while (exp) {
    if (exp & 1) {
      ret = (ret * val) % mod;
    }
    val = (val * val) % mod;
    exp = exp >> 1;
  }
  return ret;
}

uint64_t rand64() {
  _STATIC_ASSERT(RAND_MAX == 0x7fff);

  return (
    ((uint64_t)(rand()) << 49) |
    ((uint64_t)(rand()) << 34) |
    ((uint64_t)(rand()) << 19) |
    ((uint64_t)(rand()) <<  3) |
    ((uint64_t)(rand())  &  7)
  );
}

uint64_t gen_prime(uint64_t mod) {
  while (1) {
    uint64_t prime = rand64() % mod;

    for (uint64_t d = 2;; d++) {
      uint64_t d2 = d * d;
      if (d2 < d || d2 > prime) {
        return prime;
      }
      if (prime % d == 0) {
        break;
      }
    }
  }
}

void gcd_ex(uint64_t a, uint64_t b, uint64_t *x, uint64_t *y, uint64_t *gcd) {
  if (a == 0) {
    *x = 0;
    *y = 1;
    *gcd = b;
    return;
  }

  uint64_t t;
  gcd_ex(b % a, a, y, &t, gcd);
  *x = t - (*y) * (b / a);
}

void dh_example(uint64_t G, uint64_t P) {
  uint64_t a = gen_prime(P);
  uint64_t b = gen_prime(P);

  uint64_t A = pow_mod(G, a, P);
  uint64_t B = pow_mod(G, b, P);

  uint64_t ka = pow_mod(B, a, P);
  uint64_t kb = pow_mod(A, b, P);

  printf("ab: %lld, %lld, kakb: %lld, %lld\n", a, b, ka, kb);
}

int main() {
  srand(time(NULL));
  dh_example(23, gen_prime(0xffffffff));
}
