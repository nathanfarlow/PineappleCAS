#ifndef MAPPING_H_
#define MAPPING_H_

#include "../ast.h"

extern ast_t *mappings[AMOUNT_SYMBOLS];

void mapping_Init();
ast_t *mapping_Get(Symbol symbol);
void mapping_Set(Symbol symbol, ast_t *value);
void mapping_Cleanup();

#endif