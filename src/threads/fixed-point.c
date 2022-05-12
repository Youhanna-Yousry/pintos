#include "fixed-point.h"

int convert_to_int_trunc(real x){
    return x.value/ FIXED_POINT;
}

int convert_to_int_round(real x){
    int ans = 0;
    if(x.value >= 0){
        ans = (x.value + FIXED_POINT / 2) >> FIXED_POINT;
    }else{
      ans = (x.value - FIXED_POINT / 2) >> FIXED_POINT;  
    }
    return ans;
}

real* convert_to_real(int n, real *x){
    x->value = n << FIXED_POINT;
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
    ans->value = ((int64_t)(op1->value) * op2->value) / FIXED_POINT;
    return ans;
}
real* multiply_int(real *op1, int op2, real *ans){
    ans->value = op1->value * op2;
    return ans;
}

real* divide(real *op1, real *op2, real *ans){
    ans->value = ((int64_t)(op1->value) * FIXED_POINT) / op2->value;
    return ans;

}

real* divide_int(real *op1, int op2, real *ans){
    ans->value = op1->value / op2;
    return ans;
}
