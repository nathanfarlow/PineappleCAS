#ifndef NUM_H_
#define NUM_H_

#include "stdbool.h"

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

#include <stdint.h>

typedef struct _Num {
    uint16_t length;
    /*null terminated*/
    char *digits;
} num_t;

num_t *num_Create(char *digits);
num_t *num_Copy(num_t *num);
double num_ToDouble(num_t *num);
void num_Cleanup(num_t *num);

#endif