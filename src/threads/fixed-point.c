#include "fixed-point.h"

real* convert_to_real(int n, real *x);

int convert_to_int_trunc(real *x);
int convert_to_int_round(real *x);

real* add(real *op1, real *op2, real *ans);
real* subtract(real *op1, real *op2, real *ans);

real* multiply(real *op1, real *op2, real *ans);
real* multiply_int(real *op1, int op2, real *ans);

real* divide(real *op1, real *op2, real *ans);
real* divide_int(real *op1, int op2, real *ans);


/* Truncates a real number.*/
int 
convert_to_int_trunc(real *x)
{
    return x->value / F;
}

/* Rounds a real number to the nearest integer.*/
int 
convert_to_int_round(real *x)
{
    int ans = 0;
    if(x->value >= 0){
        ans = (x->value + F / 2) / F;
    }else{
      ans = (x->value - F / 2) / F;  
    }
    return ans;
}

/*Convertes an integer to a real number.*/
real* 
convert_to_real(int n, real *x)
{
    x->value = n * F;
    return x;
}
/* Adds two real numbers.*/
real* 
add(real *op1, real *op2, real *ans)
{
    ans->value = op2->value + op1->value;
    return ans;
}
/*Subtractes two real numbers (op1 - op2).*/
real* 
subtract(real *op1, real *op2, real *ans)
{
    ans->value = op1->value - op2->value;
    return ans;
}
/*multuplies two real numbers.*/
real* 
multiply(real *op1, real *op2, real *ans)
{
    ans->value = ((int64_t)(op1->value) * op2->value) / F;
    return ans;
}
/*multiplies a real number by an integer*/
real*
multiply_int(real *op1, int op2, real *ans)
{
    ans->value = op1->value * op2;
    return ans;
}
/*Divides two real numbers (op1 / op2 ).*/
real* 
divide(real *op1, real *op2, real *ans)
{
    ans->value = ((int64_t)(op1->value) * F) / op2->value;
    return ans;

}
/*Divides a real number by an integer.*/
real* 
divide_int(real *op1, int op2, real *ans)
{
    ans->value = op1->value / op2;
    return ans;
}
