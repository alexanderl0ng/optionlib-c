#include <stdio.h>
#include <math.h>
#include "optionlib.h"

#define TOL_BS  0.01
#define TOL_BT  0.10

static int g_failed = 0;

#define CHECK_CLOSE(actual, expected, tol)                                     \
    do {                                                                       \
        double _a = (actual), _e = (expected), _d = fabs(_a - _e);             \
        if (_d >= (tol)) {                                                     \
            fprintf(stderr, "  FAIL %s:%d  actual=%.6f  expected=%.6f"         \
                            "  diff=%.6f\n", __FILE__, __LINE__, _a, _e, _d);  \
            ++g_failed; return;                                                \
        }                                                                      \
    } while (0)

#define CHECK_GT(a, b)                                                         \
    do {                                                                       \
        double _a = (a), _b = (b);                                             \
        if (!(_a > _b)) {                                                      \
            fprintf(stderr, "  FAIL %s:%d  %.6f not > %.6f\n",                 \
                    __FILE__, __LINE__, _a, _b);                               \
            ++g_failed; return;                                                \
        }                                                                      \
    } while (0)


static const struct {
    double S, K, T, r, sigma, q;
    double exp_call, exp_put;
} BS_CASES[] = {
    { 100, 100, 1, 0.05, 0.2, 0.00, 10.4506,  5.5735 },
    { 110, 100, 1, 0.05, 0.2, 0.00, 17.6630,  2.7859 },
    {  90, 100, 1, 0.05, 0.2, 0.00,  5.0912, 10.2142 },
    { 100, 100, 1, 0.05, 0.2, 0.02,  9.2270,  6.3301 },
};

static void test_black_scholes_values(void)
{
    for (int i = 0; i < (int)(sizeof BS_CASES / sizeof *BS_CASES); ++i) {
        double S = BS_CASES[i].S, K = BS_CASES[i].K, T = BS_CASES[i].T;
        double r = BS_CASES[i].r, sigma = BS_CASES[i].sigma, q = BS_CASES[i].q;

        double c = q == 0.0 ? black_scholes_call(S, K, T, r, sigma)
                            : black_scholes_call_with_dividend(S, K, T, r, sigma, q);
        double p = q == 0.0 ? black_scholes_put(S, K, T, r, sigma)
                            : black_scholes_put_with_dividend(S, K, T, r, sigma, q);

        CHECK_CLOSE(c, BS_CASES[i].exp_call, TOL_BS);
        CHECK_CLOSE(p, BS_CASES[i].exp_put,  TOL_BS);
        CHECK_CLOSE(c - p, S * exp(-q * T) - K * exp(-r * T), TOL_BS);
    }
}

static void test_bs_monotonicity(void)
{
    double K = 100, T = 1, r = 0.05, sigma = 0.2;

    CHECK_GT(black_scholes_call(100, K, T, r, sigma), black_scholes_call( 90, K, T, r, sigma));
    CHECK_GT(black_scholes_call(110, K, T, r, sigma), black_scholes_call(100, K, T, r, sigma));
    CHECK_GT(black_scholes_put ( 90, K, T, r, sigma), black_scholes_put (100, K, T, r, sigma));
    CHECK_GT(black_scholes_put (100, K, T, r, sigma), black_scholes_put (110, K, T, r, sigma));
}

static void test_binomial_convergence(void)
{
    double S = 100, K = 100, T = 1, r = 0.05, sigma = 0.2, q = 0.02;

    CHECK_CLOSE(binomial_tree_call_european(S, K, T, r, sigma, 500),
                black_scholes_call(S, K, T, r, sigma), TOL_BT);
    CHECK_CLOSE(binomial_tree_put_european(S, K, T, r, sigma, 500),
                black_scholes_put(S, K, T, r, sigma), TOL_BT);
    CHECK_CLOSE(binomial_tree_call_european_with_dividend(S, K, T, r, sigma, 500, q),
                black_scholes_call_with_dividend(S, K, T, r, sigma, q), TOL_BT);
    CHECK_CLOSE(binomial_tree_put_european_with_dividend(S, K, T, r, sigma, 500, q),
                black_scholes_put_with_dividend(S, K, T, r, sigma, q), TOL_BT);

    double ref = black_scholes_call(S, K, T, r, sigma);
    int    sizes[] = {50, 100, 200, 500};
    double prev_err = 999.0;
    for (int i = 0; i < 4; ++i) {
        double err = fabs(binomial_tree_call_european(S, K, T, r, sigma, sizes[i]) - ref);
        if (err >= prev_err * 1.1) {
            fprintf(stderr, "  FAIL %s:%d  error did not shrink at step=%d"
                            " (%.6f >= %.6f)\n", __FILE__, __LINE__, sizes[i], err, prev_err);
            ++g_failed; return;
        }
        prev_err = err;
    }
}

static void test_american_vs_european(void)
{
    double S = 100, K = 100, T = 1, r = 0.05, sigma = 0.2;
    int steps = 500;

    double ec = binomial_tree_call_european(S, K, T, r, sigma, steps);
    double ac = binomial_tree_call_american(S, K, T, r, sigma, steps);
    double ep = binomial_tree_put_european (S, K, T, r, sigma, steps);
    double ap = binomial_tree_put_american (S, K, T, r, sigma, steps);

    CHECK_GT(ac + 1e-9, ec);
    CHECK_GT(ap, ep);
    CHECK_CLOSE(ac, ec, 0.05);

    CHECK_GT(binomial_tree_put_american( 50, K, T, r, sigma, steps) -
             binomial_tree_put_european(50, K, T, r, sigma, steps), 1.0);

    CHECK_GT(binomial_tree_call_american_with_dividend(S, K, T, r, sigma, steps, 0.10) -
             binomial_tree_call_european_with_dividend(S, K, T, r, sigma, steps, 0.10), 0.01);
}

static void test_american_properties(void)
{
    double K = 100, T = 1, r = 0.05, sigma = 0.2;
    int steps = 500;

    CHECK_GT(binomial_tree_call_american(100, K, T, r, sigma, steps),
             binomial_tree_call_american( 90, K, T, r, sigma, steps));
    CHECK_GT(binomial_tree_call_american(110, K, T, r, sigma, steps),
             binomial_tree_call_american(100, K, T, r, sigma, steps));
    CHECK_GT(binomial_tree_put_american( 90, K, T, r, sigma, steps),
             binomial_tree_put_american(100, K, T, r, sigma, steps));
    CHECK_GT(binomial_tree_put_american(100, K, T, r, sigma, steps),
             binomial_tree_put_american(110, K, T, r, sigma, steps));

    CHECK_GT(binomial_tree_put_american(80, K, T, 0.01, sigma, steps),
             binomial_tree_put_american(80, K, T, 0.10, sigma, steps));

    double ac = binomial_tree_call_american(100, K, T, r, sigma, steps);
    double ap = binomial_tree_put_american (100, K, T, r, sigma, steps);
    CHECK_GT(fabs((ac - ap) - (100.0 - K * exp(-r * T))), 0.1);

    CHECK_CLOSE(binomial_tree_call_american(110, K, 0.01, r, sigma, 100), 10.0, 0.1);
    CHECK_CLOSE(binomial_tree_put_american (110, K, 0.01, r, sigma, 100),  0.0, 0.1);
}

#define RUN(fn)                                                                \
    do {                                                                       \
        int _before = g_failed;                                                \
        fprintf(stderr, "[ RUN  ] " #fn "\n");                                 \
        fn();                                                                  \
        fprintf(stderr, _before == g_failed ? "[ OK   ] " #fn "\n\n"           \
                                            : "[ FAIL ] " #fn "\n\n");         \
    } while (0)

int main()
{
    printf("=== OptionLib Tests ===\n\n");

    RUN(test_black_scholes_values);
    RUN(test_bs_monotonicity);
    RUN(test_binomial_convergence);
    RUN(test_american_vs_european);
    RUN(test_american_properties);

    printf("=== %s ===\n", g_failed ? "FAILED" : "All tests passed");
    return g_failed ? 1 : 0;
}
