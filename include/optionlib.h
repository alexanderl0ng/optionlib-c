#ifndef OPTIONLIB_H
#define OPTIONLIB_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * BLACK-SCHOLES MODEL
 * ============================================================================
 * Closed-form solution for European options pricing.
 * Fast and accurate for standard European options.
 *
 * Parameters:
 *   S       - Current stock price
 *   K       - Strike price
 *   T       - Time to expiration (in years)
 *   r       - Risk-free interest rate (annual)
 *   sigma   - Volatility (annual standard deviation)
 *   dividend - Continuous dividend yield (annual)
 */

double black_scholes_call(double S, double K, double T, double r, double sigma);
double black_scholes_put(double S, double K, double T, double r, double sigma);
double black_scholes_call_with_dividend(double S, double K, double T, double r, double sigma, double dividend);
double black_scholes_put_with_dividend(double S, double K, double T, double r, double sigma, double dividend);


/* ============================================================================
 * BINOMIAL TREE MODEL
 * ============================================================================
 * Discrete-time lattice model for European options pricing.
 *
 * Parameters:
 *   S       - Current stock price
 *   K       - Strike price
 *   T       - Time to expiration (in years)
 *   r       - Risk-free interest rate (annual)
 *   sigma   - Volatility (annual standard deviation)
 *   steps   - Number of time steps in the tree (higher = more accurate but slower)
 *   dividend - Continuous dividend yield (annual)
 *
 */

double binomial_tree_call(double S, double K, double T, double r, double sigma, int steps);
double binomial_tree_put(double S, double K, double T, double r, double sigma, int steps);
double binomial_tree_call_with_dividend(double S, double K, double T, double r, double sigma, int steps, double dividend);
double binomial_tree_put_with_dividend(double S, double K, double T, double r, double sigma, int steps, double dividend);

#ifdef __cplusplus
}
#endif

#endif

