#include "optionlib.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

static double binomial_tree_calculation(double S, double K, double T, double r, double sigma, bool is_call, int steps, double dividend, bool is_american);
static inline double max_double(double x, double y);

double binomial_tree_call_european(double S, double K, double T, double r, double sigma, int steps) {
    return binomial_tree_calculation(S, K, T, r, sigma, true, steps, 0.0, false);
}

double binomial_tree_put_european(double S, double K, double T, double r, double sigma, int steps) {
    return binomial_tree_calculation(S, K, T, r, sigma, false, steps, 0.0, false);
}

double binomial_tree_call_european_with_dividend(double S, double K, double T, double r, double sigma, int steps, double dividend) {
    return binomial_tree_calculation(S, K, T, r, sigma, true, steps, dividend, false);
}

double binomial_tree_put_european_with_dividend(double S, double K, double T, double r, double sigma, int steps, double dividend) {
    return binomial_tree_calculation(S, K, T, r, sigma, false, steps, dividend, false);
}

double binomial_tree_call_american(double S, double K, double T, double r, double sigma, int steps) {
    return binomial_tree_calculation(S, K, T, r, sigma, true, steps, 0.0, true);
}

double binomial_tree_put_american(double S, double K, double T, double r, double sigma, int steps) {
    return binomial_tree_calculation(S, K, T, r, sigma, false, steps, 0.0, true);
}

double binomial_tree_call_american_with_dividend(double S, double K, double T, double r, double sigma, int steps, double dividend) {
    return binomial_tree_calculation(S, K, T, r, sigma, true, steps, dividend, true);
}

double binomial_tree_put_american_with_dividend(double S, double K, double T, double r, double sigma, int steps, double dividend) {
    return binomial_tree_calculation(S, K, T, r, sigma, false, steps, dividend, true);
}

static double binomial_tree_calculation(double S, double K, double T, double r, double sigma, bool is_call, int steps, double dividend, bool is_american) {
    double dt = T / steps;
    double u = exp(sigma * sqrt(dt));
    double d = 1 / u;
    double p = (exp((r - dividend) * dt) - d) / (u - d);
    double discount = exp(-r * dt);

    double p_disc = p * discount;
    double q_disc = (1.0 - p) * discount;

    double *u_pow = malloc((steps + 1) * sizeof(double));
    double *d_pow = malloc((steps + 1) * sizeof(double));

    u_pow[0] = 1;
    d_pow[0] = 1;

    for (int i = 1; i <= steps; i++) {
        u_pow[i] = u_pow[i - 1] * u;
	d_pow[i] = d_pow[i - 1] * d;
    }

    double *option_values = malloc((steps + 1) * sizeof(double));

    if (is_call) {
        for (int i = 0; i < steps + 1; i++) {
            option_values[i] = max_double((S * u_pow[steps - i] * d_pow[i]) - K, 0.0);
        }
    } else {
        for (int i = 0; i < steps + 1; i++) {
            option_values[i] = max_double(K - (S * u_pow[steps - i] * d_pow[i]), 0.0);
        }
    }


    if (is_american) {
        for (int i = steps - 1; i >= 0; i--) {
	        for (int j = 0; j <= i; j++) {
		        option_values[j] = p_disc * option_values[j] + q_disc * option_values[j + 1];
		        double stock_price = (S * u_pow[i - j] * d_pow[j]);
        		double intrinsic_value = is_call ?
        		    max_double(stock_price - K, 0.0) :
        		    max_double(K - stock_price, 0.0);
        		option_values[j] = max_double(option_values[j], intrinsic_value);
	    }
	}
    } else {
    	for (int i = steps - 1; i >= 0; i--) {
    	    for (int j = 0; j <= i; j++) {
    		    option_values[j] = p_disc * option_values[j] + q_disc * option_values[j + 1];
    	    }
    	}
    }

    double result = option_values[0];
    free(u_pow);
    free(d_pow);
    free(option_values);

    return result;
}

static inline double max_double(double x, double y) {
    return x > y ? x : y;
}
