/* Aria_lang/src/stdlib/math.c */
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// --- Runtime Tagging ---
typedef uint64_t Value;
#define QNAN_MASK       0x7FF8000000000000ULL
#define TAG_INTEGER     (QNAN_MASK | 0x8000000000000000ULL | 4ULL)

// Helper: Extract double from Value (handles Int and Float)
static inline double unbox(Value v) {
    if ((v & 0xFFF8000000000000ULL) == TAG_INTEGER) {
        return (double)((int32_t)(v & 0xFFFFFFFF));
    }
    union { uint64_t u; double d; } u;
    u.u = v;
    // Check if it's a valid float (not a NaN-tagged pointer)
    if ((u.u & QNAN_MASK) == QNAN_MASK) return 0.0;
    return u.d;
}

// Helper: Box double to Value
static inline Value box(double d) {
    union { double d; uint64_t u; } u;
    u.d = d;
    // If the double happens to be a NaN that looks like a tag, canonicalize it
    if ((u.u & QNAN_MASK) == QNAN_MASK) return QNAN_MASK;
    return u.u;
}

// --- Trigonometry ---
Value math_sin(Value v) { return box(sin(unbox(v))); }
Value math_cos(Value v) { return box(cos(unbox(v))); }
Value math_tan(Value v) { return box(tan(unbox(v))); }
Value math_asin(Value v) { return box(asin(unbox(v))); }
Value math_acos(Value v) { return box(acos(unbox(v))); }
Value math_atan(Value v) { return box(atan(unbox(v))); }
Value math_atan2(Value y, Value x) { return box(atan2(unbox(y), unbox(x))); }

// --- Hyperbolic ---
Value math_sinh(Value v) { return box(sinh(unbox(v))); }
Value math_cosh(Value v) { return box(cosh(unbox(v))); }
Value math_tanh(Value v) { return box(tanh(unbox(v))); }

// --- Exponentials & Logarithms ---
Value math_exp(Value v) { return box(exp(unbox(v))); }
Value math_log(Value v) { return box(log(unbox(v))); }
Value math_log10(Value v) { return box(log10(unbox(v))); }
Value math_pow(Value b, Value e) { return box(pow(unbox(b), unbox(e))); }
Value math_sqrt(Value v) { return box(sqrt(unbox(v))); }
Value math_cbrt(Value v) { return box(cbrt(unbox(v))); }

// --- Rounding & Absolute ---
Value math_ceil(Value v) { return box(ceil(unbox(v))); }
Value math_floor(Value v) { return box(floor(unbox(v))); }
Value math_round(Value v) { return box(round(unbox(v))); }
Value math_abs(Value v) { return box(fabs(unbox(v))); }

// --- Utilities ---
Value math_min(Value a, Value b) { 
    double da = unbox(a);
    double db = unbox(b);
    return box(da < db? da : db);
}

Value math_max(Value a, Value b) { 
    double da = unbox(a);
    double db = unbox(b);
    return box(da > db? da : db);
}

Value math_clamp(Value v, Value min, Value max) {
    double d = unbox(v);
    double dmin = unbox(min);
    double dmax = unbox(max);
    if (d < dmin) return box(dmin);
    if (d > dmax) return box(dmax);
    return box(d);
}

// --- Constants ---
Value math_pi() { return box(3.14159265358979323846); }
Value math_e() { return box(2.71828182845904523536); }
Value math_tau() { return box(6.28318530717958647692); }

// --- Random ---
// Seeds the random number generator
Value math_seed(Value v) {
    srand((unsigned int)unbox(v));
    return box(0);
}

// Returns random double between 0.0 and 1.0
Value math_random() {
    return box((double)rand() / (double)RAND_MAX);
}
