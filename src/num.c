#include "num.h"

#include <malloc.h>
#include <string.h>
#include <stdlib.h> /*for atof*/

num_t *num_Create(char *digits) {
    num_t *ret = malloc(sizeof(num_t));
    ret->length = (uint16_t)strlen(digits);
    ret->digits = malloc((ret->length + 1) * sizeof(char));
    memcpy(ret->digits, digits, ret->length);
    ret->digits[ret->length] = 0;
    return ret;
}

num_t *num_Copy(num_t *num) {
    return num_Create(num->digits);
}

double num_ToDouble(num_t *num) {
    char buffer[20] = {0};
    memcpy(buffer, num->digits, num->length < 19 ? num->length : 19);
    return atof(buffer);
}

void num_Cleanup(num_t *num) {
    if(num == NULL)
        return;
    if(num->length > 0)
        free(num->digits);
    free(num);
}
