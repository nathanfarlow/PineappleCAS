#include "num.h"

#include <string.h>
#include <stdlib.h> /*for atof*/

#define RADIX 10

num_t *num_CreateDecimal(char *digits) {
    mp_result res;
    num_t *ret = malloc(sizeof(num_t));

    ret->is_decimal = true;

    if((res = mp_rat_init(&ret->num.rational) != MP_OK)) {
        free(ret);
        return NULL;
    }

    if((res = mp_rat_read_decimal(&ret->num.rational, RADIX, digits) != MP_OK)) {
        free(ret);
        return NULL;
    }

    return ret;
}

num_t *num_CreateInteger(char *digits) {
    mp_result res;
    num_t *ret = malloc(sizeof(num_t));

    ret->is_decimal = false;

    if((res = mp_int_init(&ret->num.integer) != MP_OK)) {
        free(ret);
        return NULL;
    }
    if((res = mp_int_read_string(&ret->num.integer, RADIX, digits)) != MP_OK) {
        free(ret);
        return NULL;
    }

    return ret;
}

num_t *num_Copy(num_t *num) {
    mp_result res;

    num_t *ret = malloc(sizeof(num_t));
    ret->is_decimal = num->is_decimal;

    if(ret->is_decimal) {
        if((res = mp_rat_init_copy(&ret->num.rational, &num->num.rational)) != MP_OK) {
            free(ret);
            return NULL;
        }
    } else {
        if((res = mp_int_init_copy(&ret->num.integer, &num->num.integer)) != MP_OK) {
            free(ret);
            return NULL;
        }
    }

    return ret;
}

double num_ToDouble(num_t *num) {
    char buffer[50] = { 0 };

    if(num->is_decimal) {
        mp_rat_to_decimal(&num->num.rational, RADIX, 16, MP_ROUND_HALF_UP, buffer, sizeof(buffer));
    } else {
        mp_int_to_string(&num->num.integer, RADIX, buffer, sizeof(buffer));
    }

    return atof(buffer);
}

char *num_ToString(num_t *num, mp_size precision) {
    mp_result res;

    char *str;
    int len;

    if(num->is_decimal) {
        len = mp_rat_decimal_len(&num->num.rational, RADIX, precision);
        str = malloc(len * sizeof(char));
        if((res = mp_rat_to_decimal(&num->num.rational, RADIX, precision, MP_ROUND_HALF_UP, str, len)) != MP_OK) {
            free(str);
            return NULL;
        }
    } else {
        len = mp_int_string_len(&num->num.integer, RADIX);
        str = malloc(len * sizeof(char));
        if((res = mp_int_to_string(&num->num.integer, RADIX, str, len)) != MP_OK) {
            free(str);
            return NULL;
        }
    }

    return str;
}

void num_Cleanup(num_t *num) {
    if(num == NULL)
        return;

    if(num->is_decimal) {
        mp_rat_clear(&num->num.rational);
    } else {
        mp_int_clear(&num->num.integer);
    }

    free(num);
}
