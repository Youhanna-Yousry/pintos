#include <stdint.h>
#define FIXED_POINT 14

typedef struct {
    int value;
} real;

int convert_to_int_trunc(real *x);

int convert_to_int_round(real *x);

real* convert_to_real(int n, real *x);
real* add(real *op1, real *op2, real *ans);
real* subtract(real *op1, real *op2, real *ans);
real* multiply(real *op1, real *op2, real *ans);
real* multiply_int(real *op1, int op2, real *ans);
real* divide(real *op1, real *op2, real *ans);
real* divide_int(real *op1, int op2, real *ans);






