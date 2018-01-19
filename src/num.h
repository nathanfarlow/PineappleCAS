#ifndef NUM_H_
#define NUM_H_

#include <stdbool.h>
#include <stdint.h>

#include "imath/imath.h"
#include "imath/imrat.h"

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