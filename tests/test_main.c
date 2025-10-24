#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "optionlib.h"

#define TOLERANCE 0.01
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

    printf("--- TEST 1: Black-Scholes ATM (S=K=100) ---\n");
    double S = 100.0, K = 100.0, T = 1.0, r = 0.05, sigma = 0.2;

    double bs_call_atm = black_scholes_call(S, K, T, r, sigma);
    double bs_put_atm = black_scholes_put(S, K, T, r, sigma);

    assert_close(bs_call_atm, 10.4506, TOLERANCE, "BS Call ATM");
    assert_close(bs_put_atm, 5.5735, TOLERANCE, "BS Put ATM");
    assert_parity(bs_call_atm, bs_put_atm, S, K, r, T, 0.0, "BS Put-Call Parity (no div)");

    printf("\n--- TEST 2: Black-Scholes ITM Call (S=110, K=100) ---\n");
    S = 110.0; K = 100.0;
    double bs_call_itm = black_scholes_call(S, K, T, r, sigma);
    double bs_put_otm = black_scholes_put(S, K, T, r, sigma);

    assert_close(bs_call_itm, 17.6630, TOLERANCE, "BS Call ITM");
    assert_close(bs_put_otm, 2.7859, TOLERANCE, "BS Put OTM");
    assert_parity(bs_call_itm, bs_put_otm, S, K, r, T, 0.0, "BS Put-Call Parity ITM/OTM");

    printf("\n--- TEST 3: Black-Scholes OTM Call (S=90, K=100) ---\n");
    S = 90.0; K = 100.0;
    double bs_call_otm = black_scholes_call(S, K, T, r, sigma);
    double bs_put_itm = black_scholes_put(S, K, T, r, sigma);

    assert_close(bs_call_otm, 5.0912, TOLERANCE, "BS Call OTM");
    assert_close(bs_put_itm, 10.2142, TOLERANCE, "BS Put ITM");

    printf("\n--- TEST 4: Black-Scholes with Dividends (q=0.02) ---\n");
    S = 100.0; K = 100.0;
    double dividend = 0.02;
    double bs_call_div = black_scholes_call_with_dividend(S, K, T, r, sigma, dividend);
    double bs_put_div = black_scholes_put_with_dividend(S, K, T, r, sigma, dividend);

    assert_close(bs_call_div, 9.2270, TOLERANCE, "BS Call with Dividend");
    assert_close(bs_put_div, 6.3301, TOLERANCE, "BS Put with Dividend");
    assert_parity(bs_call_div, bs_put_div, S, K, r, T, dividend, "BS Put-Call Parity (with div)");

    printf("\n--- TEST 5: Option Price Monotonicity ---\n");
    S = 100.0; K = 100.0; dividend = 0.0;

    double call_90 = black_scholes_call(90, K, T, r, sigma);
    double call_100 = black_scholes_call(100, K, T, r, sigma);
    double call_110 = black_scholes_call(110, K, T, r, sigma);
    assert_greater(call_100, call_90, "Call increases with S (100 > 90)");
    assert_greater(call_110, call_100, "Call increases with S (110 > 100)");

    double put_90 = black_scholes_put(90, K, T, r, sigma);
    double put_100 = black_scholes_put(100, K, T, r, sigma);
    double put_110 = black_scholes_put(110, K, T, r, sigma);
    assert_greater(put_90, put_100, "Put decreases with S (90 > 100)");
    assert_greater(put_100, put_110, "Put decreases with S (100 > 110)");

    printf("\n--- TEST 6: Binomial Tree Convergence ---\n");
    S = 100.0; K = 100.0;
    int steps = 500;

    double bt_call = binomial_tree_call_european(S, K, T, r, sigma, steps);
    double bt_put = binomial_tree_put_european(S, K, T, r, sigma, steps);
    double bs_call_ref = black_scholes_call(S, K, T, r, sigma);
    double bs_put_ref = black_scholes_put(S, K, T, r, sigma);

    assert_close(bt_call, bs_call_ref, 0.1, "Binomial Call vs BS (500 steps)");
    assert_close(bt_put, bs_put_ref, 0.1, "Binomial Put vs BS (500 steps)");

    printf("\n--- TEST 7: Binomial Tree with Dividends ---\n");
    dividend = 0.02;
    double bt_call_div = binomial_tree_call_european_with_dividend(S, K, T, r, sigma, steps, dividend);
    double bt_put_div = binomial_tree_put_european_with_dividend(S, K, T, r, sigma, steps, dividend);
    double bs_call_div_ref = black_scholes_call_with_dividend(S, K, T, r, sigma, dividend);
    double bs_put_div_ref = black_scholes_put_with_dividend(S, K, T, r, sigma, dividend);

    assert_close(bt_call_div, bs_call_div_ref, 0.1, "Binomial Call with Div vs BS");
    assert_close(bt_put_div, bs_put_div_ref, 0.1, "Binomial Put with Div vs BS");

    printf("\n--- TEST 8: Binomial Convergence Rate ---\n");
    bs_call_ref = black_scholes_call(100, 100, 1, 0.05, 0.2);
    int step_sizes[] = {10, 50, 100, 200, 500};
    double prev_error = 999.0;

    printf("\n--- TEST 9: American vs European Options (ATM) ---\n");
    S = 100.0; K = 100.0; T = 1.0; r = 0.05; sigma = 0.2; dividend = 0.0;
    steps = 500;

    double euro_call = binomial_tree_call_european(S, K, T, r, sigma, steps);
    double amer_call = binomial_tree_call_american(S, K, T, r, sigma, steps);
    double euro_put = binomial_tree_put_european(S, K, T, r, sigma, steps);
    double amer_put = binomial_tree_put_american(S, K, T, r, sigma, steps);

    assert_greater(amer_call + 0.0001, euro_call, "American Call >= European Call (ATM, no div)");
    assert_greater(amer_put, euro_put, "American Put > European Put (ATM)");

    printf("European Call: %.4f, American Call: %.4f (diff: %.4f)\n",
           euro_call, amer_call, amer_call - euro_call);
    printf("European Put: %.4f, American Put: %.4f (diff: %.4f)\n",
           euro_put, amer_put, amer_put - euro_put);

    printf("\n--- TEST 10: American Put Early Exercise Premium (Deep ITM) ---\n");
    S = 50.0; K = 100.0; T = 1.0; r = 0.05; sigma = 0.2; dividend = 0.0;

    euro_put = binomial_tree_put_european(S, K, T, r, sigma, steps);
    amer_put = binomial_tree_put_american(S, K, T, r, sigma, steps);

    double put_premium = amer_put - euro_put;

    printf("European Put: %.4f, American Put: %.4f\n", euro_put, amer_put);
    printf("Early Exercise Premium: %.4f\n", put_premium);

    assert_greater(amer_put, euro_put, "American Put > European Put (Deep ITM)");
    assert_greater(put_premium, 1.0, "Early Exercise Premium > $1.00");

    printf("\n--- TEST 11: American Call with High Dividends ---\n");
    S = 100.0; K = 100.0; T = 1.0; r = 0.05; sigma = 0.2; dividend = 0.10;

    euro_call = binomial_tree_call_european_with_dividend(S, K, T, r, sigma, steps, dividend);
    amer_call = binomial_tree_call_american_with_dividend(S, K, T, r, sigma, steps, dividend);

    double call_premium = amer_call - euro_call;

    printf("European Call: %.4f, American Call: %.4f\n", euro_call, amer_call);
    printf("Early Exercise Premium: %.4f\n", call_premium);

    assert_greater(amer_call, euro_call, "American Call > European Call (High Dividend)");
    assert_greater(call_premium, 0.01, "Call Premium > $0.01 (High Dividend)");

    printf("\n--- TEST 12: American Call No Dividends (Should ≈ European) ---\n");
    S = 100.0; K = 100.0; T = 1.0; r = 0.05; sigma = 0.2; dividend = 0.0;

    euro_call = binomial_tree_call_european(S, K, T, r, sigma, steps);
    amer_call = binomial_tree_call_american(S, K, T, r, sigma, steps);

    assert_close(amer_call, euro_call, 0.05, "American Call ≈ European Call (No Dividends)");

    printf("\n--- TEST 13: American Options at Expiration (T→0) ---\n");
    S = 110.0; K = 100.0; T = 0.01; r = 0.05; sigma = 0.2; dividend = 0.0;

    amer_call = binomial_tree_call_american(S, K, T, r, sigma, 100);
    amer_put = binomial_tree_put_american(S, K, T, r, sigma, 100);

    double intrinsic_call = fmax(S - K, 0.0);
    double intrinsic_put = fmax(K - S, 0.0);

    assert_close(amer_call, intrinsic_call, 0.1, "American Call → Intrinsic (T→0)");
    assert_close(amer_put, intrinsic_put, 0.1, "American Put → Intrinsic (T→0)");

    printf("\n--- TEST 14: American Put-Call Parity Violation ---\n");
    S = 100.0; K = 100.0; T = 1.0; r = 0.05; sigma = 0.2; dividend = 0.0;

    amer_call = binomial_tree_call_american(S, K, T, r, sigma, steps);
    amer_put = binomial_tree_put_american(S, K, T, r, sigma, steps);

    double lhs = amer_call - amer_put;
    double rhs = S * exp(-dividend * T) - K * exp(-r * T);
    double parity_diff = fabs(lhs - rhs);

    printf("C - P = %.4f\n", lhs);
    printf("S*e^(-qT) - K*e^(-rT) = %.4f\n", rhs);
    printf("Difference: %.4f\n", parity_diff);

    if (parity_diff > 0.1) {
        printf("%s Put-Call Parity properly violated for American options (diff: %.4f)\n",
               TEST_PASS, parity_diff);
        tests_passed++;
    } else {
        printf("%s Warning: American options too close to parity (diff: %.4f)\n",
               TEST_FAIL, parity_diff);
        tests_failed++;
    }

    printf("\n--- TEST 15: High Interest Rate Effect on American Put ---\n");
    S = 80.0; K = 100.0; T = 1.0; sigma = 0.2; dividend = 0.0;

    double amer_put_low_r = binomial_tree_put_american(S, K, T, 0.01, sigma, steps);
    double amer_put_high_r = binomial_tree_put_american(S, K, T, 0.10, sigma, steps);

    printf("American Put (r=1%%): %.4f\n", amer_put_low_r);
    printf("American Put (r=10%%): %.4f\n", amer_put_high_r);

    assert_greater(amer_put_low_r, amer_put_high_r, "Put value decreases with higher r");

    printf("\n--- TEST 16: American Option Monotonicity ---\n");
    K = 100.0; T = 1.0; r = 0.05; sigma = 0.2; dividend = 0.0;

    double amer_call_90 = binomial_tree_call_american(90, K, T, r, sigma, steps);
    double amer_call_100 = binomial_tree_call_american(100, K, T, r, sigma, steps);
    double amer_call_110 = binomial_tree_call_american(110, K, T, r, sigma, steps);

    assert_greater(amer_call_100, amer_call_90, "American Call increases with S (100 > 90)");
    assert_greater(amer_call_110, amer_call_100, "American Call increases with S (110 > 100)");

    double amer_put_90 = binomial_tree_put_american(90, K, T, r, sigma, steps);
    double amer_put_100 = binomial_tree_put_american(100, K, T, r, sigma, steps);
    double amer_put_110 = binomial_tree_put_american(110, K, T, r, sigma, steps);

    assert_greater(amer_put_90, amer_put_100, "American Put decreases with S (90 > 100)");
    assert_greater(amer_put_100, amer_put_110, "American Put decreases with S (100 > 110)");

    printf("Steps | Binomial Value | BS Value | Error\n");
    printf("------|----------------|----------|-------\n");
    for (int i = 0; i < 5; i++) {
        double bt_val = binomial_tree_call_european(100, 100, 1, 0.05, 0.2, step_sizes[i]);
        double error = fabs(bt_val - bs_call_ref);
        printf("%5d | %14.4f | %8.4f | %.6f\n", step_sizes[i], bt_val, bs_call_ref, error);

        if (i > 2) {
            if (error < prev_error * 1.1) {
                tests_passed++;
            } else {
                tests_failed++;
            }
        }
        prev_error = error;
    }
    
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
