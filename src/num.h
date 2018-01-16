#ifndef NUM_H_
#define NUM_H_

#include <stdbool.h>
#include <stdint.h>

#include "imath/imath.h"
#include "imath/imrat.h"

/*
A note:
I wasn't originally planning to have the num_t struct always be treated using pointers,
but there is a bug with the ez80 compiler that disallows nested functions that return
non pointers.
What I mean is, I wanted to write num_t num_Create() instead of num_t *num_Create().
Is there a speed difference I wonder? Is passing it by pointer everywhere faster anyway?

Maybe someday we will write a bignum library out of this num_t struct to aid in the
simplification process. I don't know if that's really practical on the calculator, though.
*/

typedef struct _Num {
    bool is_decimal;
    union {
        mpq_t rational;
        mpz_t integer;
    } num;
} num_t;

num_t *num_CreateDecimal(char *digits);
num_t *num_CreateInteger(char *digits);
num_t *num_Copy(num_t *num);
double num_ToDouble(num_t *num);
char *num_ToString(num_t *num, mp_size precision);
void num_Cleanup(num_t *num);

#endif