#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const unsigned long long MAX_SIZE       = 20'000;
static const unsigned long long MAX_ITERATIONS = 1'000;
static const long double        SLOPE          = 104.76415178263481;
static const long double        INTERCEPT      = 2.76415178263481;
static const long double        LEARNING_RATE  = 0.175000;
static const long double        RAND_MAXF      = 32767.0L; // (long double) RAND_MAX

static inline long double* generate_x(register const size_t length) {
    long double* array = malloc(length * sizeof(long double));
    if (!array) {
        fprintf(stderr, "memory allocation failure @ line %d inside function %s\n", __LINE__, __FUNCTION__);
        return NULL;
    }
    for (size_t i = 0; i < length; ++i) array[i] = rand() / RAND_MAXF;
    return array;
}

static inline long double* generate_y(
    register const long double* const x, register const size_t length, register const long double a, register const long double b
) {
    long double* array = malloc(length * sizeof(long double));
    if (!array) {
        fprintf(stderr, "memory allocation failure @ line %d inside function %s\n", __LINE__, __FUNCTION__);
        return NULL;
    }
    for (size_t i = 0; i < length; ++i) array[i] = x[i] * a + b;
    return array;
}

static  long double  loss( // squared error / 2
     register const long double x,
     register const long double y,
     register const long double a,
     register const long double b
) {
    return powl(y - (a * x + b), 2.00L) / 2.0000L;
}

static inline long double cost(
    const long double* const   x,
    const long double* const   y,
    register const size_t      length,
    register const long double a,
    register const long double b
) {
    // loss = (prediction - target) ^ 2 / 2
    // cost is averaged aggregate loss
    long double cost = 0.0000;
    for (size_t i = 0; i < length; ++i) cost += loss(x[i], y[i], a, b); // loss aggregation
    return cost / length;                                               // averaging
}

// but the derivative of the cost function with respect to Y_hat is -(Y - Y_hat)
// which makes all the squaring operations unnecessary!

typedef struct pair {
        long double value_0;
        long double value_1;
} pair_t;

static inline pair_t deriv_cost(
    register const long double* const x,
    register const long double* const y,
    register const size_t             length,
    register const long double        a,
    register const long double        b
) {
    // derivative of the cost function with respect to the weight and bias (i.e. our prediction)
    long double dcost_w = 0.0000, dcost_b = 0.0000, dcost = 0.000;

    for (size_t i = 0; i < length; ++i) {
        dcost    = y[i] - (x[i] * a + b); //  (Y_i - Y_hat_i)
        dcost_b -= dcost;                 // -(Y_i - Y_hat_i)
        dcost_w -= (dcost * x[i]);        // -(Y_i - Y_hat_i) * x_i
    }

    return (pair_t) { dcost_w / length, dcost_b / length }; // averaging
}

int main(void) {
    srand((unsigned) time(NULL));

    const long double* const predictor = generate_x(MAX_SIZE);
    const long double* const target    = generate_y(predictor, MAX_SIZE, SLOPE, INTERCEPT);
    assert(predictor);
    assert(target);

    // boostrap a and b with two random values between 0.0 and 1.0
    long double learned_a = rand() / RAND_MAXF, learned_b = rand() / RAND_MAXF;
    printf("bootstrapped a = %2.10Lf, b = %2.10Lf\n", learned_a, learned_b);
    pair_t derror = { 0.000, 0.000 };

    // gradient descent
    for (size_t i = 0; i < MAX_ITERATIONS; ++i) {
        // calculate the derivatives and update learned_a and learned_b
        derror     = deriv_cost(predictor, target, MAX_SIZE, learned_a, learned_b);
        learned_a -= derror.value_0 * LEARNING_RATE;
        learned_b -= derror.value_1 * LEARNING_RATE;
    }

    free(predictor);
    free(target);

    printf("actual       a = %2.10Lf, b = %2.10Lf\n", SLOPE, INTERCEPT);
    printf("inferred     a = %2.10Lf, b = %2.10Lf\n", learned_a, learned_b);

    return EXIT_SUCCESS;
}
