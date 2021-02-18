#include <stdio.h>
#include <stdint.h>
#include <math.h>

const double EPSILON = 0.001;

double ln_approx_series(double x, uint32_t n) {
	double temp = ((n & 1) ? 1.0 : -1.0) * pow(x - 1, (double)n) / (double) n;
	if (fabs(temp) >= EPSILON) {
		temp += ln_approx_series(x, n + 1);
	}
	return temp;
}

double ln_approx(double x) {
	return ln_approx_series(x, 1);
}

int main(int argc, char** argv) {
	double x;

	printf("Enter X to calculate Ln(X): ");

	scanf("%lf", &x);

	printf("Ln(%.4f) ~= %.4f\n", x, ln_approx(x));

	return 0;
}
