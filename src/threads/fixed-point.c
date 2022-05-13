#include "fixed-point.h"

int f = 1 << FIXED_POINT;

int convert_to_int_trunc(real *x){
    return x->value / f;
}

int convert_to_int_round(real *x){
    int ans = 0;
    if(x->value >= 0){
        ans = (x->value + f / 2) / f;
    }else{
      ans = (x->value - f / 2) / f;  
    }
    return ans;
}

real* convert_to_real(int n, real *x){
    x->value = n * f;
    return x;
}

real* add(real *op1, real *op2, real *ans){
    ans->value = op2->value + op1->value;
    return ans;
}

real* subtract(real *op1, real *op2, real *ans){
    ans->value = op1->value - op2->value;
    return ans;
}

real* multiply(real *op1, real *op2, real *ans){
    ans->value = ((int64_t)(op1->value) * op2->value) / f;
    return ans;
}
real* multiply_int(real *op1, int op2, real *ans){
    ans->value = op1->value * op2;
    return ans;
}

real* divide(real *op1, real *op2, real *ans){
    ans->value = ((int64_t)(op1->value) * f) / op2->value;
    return ans;

}

real* divide_int(real *op1, int op2, real *ans){
    ans->value = op1->value / op2;
    return ans;
}
