#ifndef EXPORT_H_
#define EXPORT_H_

#include <stdint.h>

#include "ast.h"

uint8_t *export_to_binary(ast_t *e, unsigned *len, error *err);

#endif