#include "optionlib.h"
#include <math.h>
#include <stdbool.h>

#define SQRT2   1.4142135623730951

static double black_scholes_calculation(double S, double K, double T, double r, double sigma, bool is_call, double dividend);
static double phi(double x);

double black_scholes_call(double S, double K, double T, double r, double sigma) {
    return black_scholes_calculation(S, K, T, r, sigma, true, 0.0);
}

double black_scholes_put(double S, double K, double T, double r, double sigma) {
    return black_scholes_calculation(S, K, T, r, sigma, false, 0.0);
}

double black_scholes_call_with_dividend(double S, double K, double T, double r, double sigma, double dividend) {
    return black_scholes_calculation(S, K, T, r, sigma, true, dividend);
}

double black_scholes_put_with_dividend(double S, double K, double T, double r, double sigma, double dividend) {
    return black_scholes_calculation(S, K, T, r, sigma, false, dividend);
}

static double black_scholes_calculation(double S, double K, double T, double r, double sigma, bool is_call, double dividend) {
    double sqrt_T = sqrt(T);
    double d_1 = (log(S / K) + T * (r - dividend + ((sigma * sigma) / 2))) / (sigma * sqrt_T);
    double d_2 = d_1 - (sigma * sqrt_T);
    double exp_rT = exp(-r * T);
    double exp_qt = exp(-dividend * T);

    double option_value;

    if (is_call) {
        option_value = (S * exp_qt * phi(d_1)) - (K * exp_rT * phi(d_2));
    } else {
        option_value = (K * exp_rT * phi(-d_2)) - (S * exp_qt * phi(-d_1));
    }

    return option_value;
}

static double phi(double x) {
    return 0.5 * (1 + erf(x / SQRT2));
}
