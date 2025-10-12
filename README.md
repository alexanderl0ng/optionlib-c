# OptionLib-C

A high-performance options pricing library written in C, compiled to WebAssembly.

---

## Features

- **Black-Scholes Model:** Analytical solution for European options
- **Binomial Tree Model:** Discrete-time pricing model
- **Dividend Support:** Both models support continuous dividend yields
- **WebAssembly:** Compile to WASM for use in web applications

---

## Building

### Native Build

```bash
./build.sh test    # Build and run tests
./build.sh lib     # Build static library
```

### WebAssembly Build

Requires [Emscripten](https://emscripten.org/).

```bash
./build_wasm.sh          # Release build
./build_wasm.sh debug    # Debug build
```

---

## API

### Black-Scholes

```c
double black_scholes_call(double S, double K, double T, double r, double sigma);
double black_scholes_put(double S, double K, double T, double r, double sigma);
double black_scholes_call_with_dividend(double S, double K, double T, double r, double sigma, double dividend);
double black_scholes_put_with_dividend(double S, double K, double T, double r, double sigma, double dividend);
```

### Binomial Tree

```c
double binomial_tree_call(double S, double K, double T, double r, double sigma, int steps);
double binomial_tree_put(double S, double K, double T, double r, double sigma, int steps);
double binomial_tree_call_with_dividend(double S, double K, double T, double r, double sigma, int steps, double dividend);
double binomial_tree_put_with_dividend(double S, double K, double T, double r, double sigma, int steps, double dividend);
```

---

## Parameters

`S`: Current stock/asset price   
`K`: Strike price   
`T`: Time to expiration (years)   
`r`: Risk-free interest rate (annual)    
`sigma`: Volatility (annual standard deviation)    
`dividend`: Continuous dividend yield (annual)    
`steps`: Number of time steps in binomial tree    


