#include <stdint.h>
#define FIXED_POINT 14

#define F (1 << FIXED_POINT)

typedef struct {
    int value;
} real;

/********************************************************
 *    A Fixed Point representation, using an integer.   *
 *                     1 sign bit,                      *
 *             17 bits for the integer part,            *
 *              14 bits for the decimal part.           *
 * **************************************************** */

real* convert_to_real(int n, real *x);

int convert_to_int_trunc(real *x);
int convert_to_int_round(real *x);

real* add(real *op1, real *op2, real *ans);
real* subtract(real *op1, real *op2, real *ans);

real* multiply(real *op1, real *op2, real *ans);
real* multiply_int(real *op1, int op2, real *ans);

real* divide(real *op1, real *op2, real *ans);
real* divide_int(real *op1, int op2, real *ans);






