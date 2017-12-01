#include "num.h"

#include <malloc.h>
#include <string.h>

num_t *num_Create(char *digits) {
	num_t *ret = malloc(sizeof(num_t));
	ret->length = (uint16_t)strlen(digits);
	ret->digits = malloc(ret->length);
	memcpy(ret->digits, digits, ret->length);
	return ret;
}

num_t *num_Copy(num_t *num) {
	return num_Create(num->digits);
}

void num_Cleanup(num_t *num) {
	if(num == NULL)
		return;
	if(num->length > 0)
		free(num->digits);
	free(num);
}
