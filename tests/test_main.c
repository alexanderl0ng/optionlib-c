#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "optionlib.h"

#define TOLERANCE 0.01  // Slightly relaxed for numerical precision
#define TEST_PASS "\033[32m[PASS]\033[0m"
#define TEST_FAIL "\033[31m[FAIL]\033[0m"

int tests_passed = 0;
int tests_failed = 0;

bool assert_close(double actual, double expected, double tolerance, const char* test_name) {
    double diff = fabs(actual - expected);
    bool passed = diff < tolerance;

    if (passed) {
        printf("%s %s: %.4f (expected %.4f, diff: %.6f)\n",
               TEST_PASS, test_name, actual, expected, diff);
        tests_passed++;
    } else {
        printf("%s %s: %.4f (expected %.4f, diff: %.6f)\n",
               TEST_FAIL, test_name, actual, expected, diff);
        tests_failed++;
    }

    return passed;
}

bool assert_parity(double call, double put, double S, double K, double r, double T, double dividend, const char* test_name) {
    double lhs = call - put;
    double rhs = S * exp(-dividend * T) - K * exp(-r * T);
    double diff = fabs(lhs - rhs);
    bool passed = diff < TOLERANCE;

    if (passed) {
        printf("%s %s: C-P = %.4f, S*e^(-qT)-K*e^(-rT) = %.4f (diff: %.6f)\n",
               TEST_PASS, test_name, lhs, rhs, diff);
        tests_passed++;
    } else {
        printf("%s %s: C-P = %.4f, S*e^(-qT)-K*e^(-rT) = %.4f (diff: %.6f)\n",
               TEST_FAIL, test_name, lhs, rhs, diff);
        tests_failed++;
    }

    return passed;
}

bool assert_greater(double value1, double value2, const char* test_name) {
    bool passed = value1 > value2;

    if (passed) {
        printf("%s %s: %.4f > %.4f\n",
               TEST_PASS, test_name, value1, value2);
        tests_passed++;
    } else {
        printf("%s %s: %.4f NOT > %.4f\n",
               TEST_FAIL, test_name, value1, value2);
        tests_failed++;
    }

    return passed;
}

int main() {
    printf("=== OptionLib Test Suite ===\n\n");

    // Test 1: Black-Scholes ATM (At-The-Money)
    printf("--- TEST 1: Black-Scholes ATM (S=K=100) ---\n");
    double S = 100.0, K = 100.0, T = 1.0, r = 0.05, sigma = 0.2;

    double bs_call_atm = black_scholes_call(S, K, T, r, sigma);
    double bs_put_atm = black_scholes_put(S, K, T, r, sigma);

    // Verified Black-Scholes values for S=100, K=100, T=1, r=0.05, σ=0.2
    assert_close(bs_call_atm, 10.4506, TOLERANCE, "BS Call ATM");
    assert_close(bs_put_atm, 5.5735, TOLERANCE, "BS Put ATM");
    assert_parity(bs_call_atm, bs_put_atm, S, K, r, T, 0.0, "BS Put-Call Parity (no div)");

    // Test 2: Black-Scholes ITM Call (In-The-Money: S > K)
    printf("\n--- TEST 2: Black-Scholes ITM Call (S=110, K=100) ---\n");
    S = 110.0; K = 100.0;
    double bs_call_itm = black_scholes_call(S, K, T, r, sigma);
    double bs_put_otm = black_scholes_put(S, K, T, r, sigma);

    // Verified values for S=110, K=100, T=1, r=0.05, σ=0.2
    assert_close(bs_call_itm, 17.6630, TOLERANCE, "BS Call ITM");
    assert_close(bs_put_otm, 2.7859, TOLERANCE, "BS Put OTM");
    assert_parity(bs_call_itm, bs_put_otm, S, K, r, T, 0.0, "BS Put-Call Parity ITM/OTM");

    // Test 3: Black-Scholes OTM Call (Out-of-The-Money: S < K)
    printf("\n--- TEST 3: Black-Scholes OTM Call (S=90, K=100) ---\n");
    S = 90.0; K = 100.0;
    double bs_call_otm = black_scholes_call(S, K, T, r, sigma);
    double bs_put_itm = black_scholes_put(S, K, T, r, sigma);

    // Verified values for S=90, K=100, T=1, r=0.05, σ=0.2
    assert_close(bs_call_otm, 5.0912, TOLERANCE, "BS Call OTM");
    assert_close(bs_put_itm, 10.2142, TOLERANCE, "BS Put ITM");

    // Test 4: Black-Scholes with Dividends
    printf("\n--- TEST 4: Black-Scholes with Dividends (q=0.02) ---\n");
    S = 100.0; K = 100.0;
    double dividend = 0.02;
    double bs_call_div = black_scholes_call_with_dividend(S, K, T, r, sigma, dividend);
    double bs_put_div = black_scholes_put_with_dividend(S, K, T, r, sigma, dividend);

    // Verified values for S=100, K=100, T=1, r=0.05, σ=0.2, q=0.02
    assert_close(bs_call_div, 9.2270, TOLERANCE, "BS Call with Dividend");
    assert_close(bs_put_div, 6.3301, TOLERANCE, "BS Put with Dividend");
    assert_parity(bs_call_div, bs_put_div, S, K, r, T, dividend, "BS Put-Call Parity (with div)");

    // Test 5: Monotonicity Tests (Logical Relationships)
    printf("\n--- TEST 5: Option Price Monotonicity ---\n");
    S = 100.0; K = 100.0; dividend = 0.0;

    // Call should increase with stock price
    double call_90 = black_scholes_call(90, K, T, r, sigma);
    double call_100 = black_scholes_call(100, K, T, r, sigma);
    double call_110 = black_scholes_call(110, K, T, r, sigma);
    assert_greater(call_100, call_90, "Call increases with S (100 > 90)");
    assert_greater(call_110, call_100, "Call increases with S (110 > 100)");

    // Put should decrease with stock price
    double put_90 = black_scholes_put(90, K, T, r, sigma);
    double put_100 = black_scholes_put(100, K, T, r, sigma);
    double put_110 = black_scholes_put(110, K, T, r, sigma);
    assert_greater(put_90, put_100, "Put decreases with S (90 > 100)");
    assert_greater(put_100, put_110, "Put decreases with S (100 > 110)");

    // Test 6: Binomial Tree vs Black-Scholes Convergence
    printf("\n--- TEST 6: Binomial Tree Convergence ---\n");
    S = 100.0; K = 100.0;
    int steps = 500;  // High steps for convergence

    double bt_call = binomial_tree_call(S, K, T, r, sigma, steps);
    double bt_put = binomial_tree_put(S, K, T, r, sigma, steps);
    double bs_call_ref = black_scholes_call(S, K, T, r, sigma);
    double bs_put_ref = black_scholes_put(S, K, T, r, sigma);

    // Binomial should be close to BS with high steps (within 0.1)
    assert_close(bt_call, bs_call_ref, 0.1, "Binomial Call vs BS (500 steps)");
    assert_close(bt_put, bs_put_ref, 0.1, "Binomial Put vs BS (500 steps)");

    // Test 7: Binomial Tree with Dividends
    printf("\n--- TEST 7: Binomial Tree with Dividends ---\n");
    dividend = 0.02;
    double bt_call_div = binomial_tree_call_with_dividend(S, K, T, r, sigma, steps, dividend);
    double bt_put_div = binomial_tree_put_with_dividend(S, K, T, r, sigma, steps, dividend);
    double bs_call_div_ref = black_scholes_call_with_dividend(S, K, T, r, sigma, dividend);
    double bs_put_div_ref = black_scholes_put_with_dividend(S, K, T, r, sigma, dividend);

    assert_close(bt_call_div, bs_call_div_ref, 0.1, "Binomial Call with Div vs BS");
    assert_close(bt_put_div, bs_put_div_ref, 0.1, "Binomial Put with Div vs BS");

    // Test 8: Binomial Convergence Analysis (showing error decreases)
    printf("\n--- TEST 8: Binomial Convergence Rate ---\n");
    bs_call_ref = black_scholes_call(100, 100, 1, 0.05, 0.2);
    int step_sizes[] = {10, 50, 100, 200, 500};
    double prev_error = 999.0;

    printf("Steps | Binomial Value | BS Value | Error\n");
    printf("------|----------------|----------|-------\n");
    for (int i = 0; i < 5; i++) {
        double bt_val = binomial_tree_call(100, 100, 1, 0.05, 0.2, step_sizes[i]);
        double error = fabs(bt_val - bs_call_ref);
        printf("%5d | %14.4f | %8.4f | %.6f\n", step_sizes[i], bt_val, bs_call_ref, error);

        // Error should generally decrease (allow some numerical noise)
        if (i > 2) {  // Only check after 100 steps
            if (error < prev_error * 1.1) {  // Allow 10% tolerance
                tests_passed++;
            } else {
                tests_failed++;
            }
        }
        prev_error = error;
    }
    
    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total: %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\n%s All tests passed!\n", TEST_PASS);
        return 0;
    } else {
        printf("\n%s Some tests failed.\n", TEST_FAIL);
        return 1;
    }
}
