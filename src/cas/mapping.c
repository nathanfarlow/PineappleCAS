#include "mapping.h"

ast_t *mappings[AMOUNT_SYMBOLS] = {0};

void mapping_Init() {
	mappings[SYM_PI] = ast_MakeNumber(num_Create("3.141592653589793"));
 	mappings[SYM_EULER] = ast_MakeNumber(num_Create("2.718281828459045"));
}

ast_t *mapping_Get(Symbol symbol) {
	uint8_t index = symbol <= SYM_THETA ? symbol : symbol - 'A';
	return mappings[index];
}

void mapping_Set(Symbol symbol, ast_t *value) {
	uint8_t index = symbol <= SYM_THETA ? symbol : symbol - 'A';
	ast_Cleanup(mappings[index]);
	mappings[index] = value;
}

void mapping_Cleanup() {
	uint8_t i;
	for(i = 0; i < AMOUNT_SYMBOLS; i++)
		ast_Cleanup(mappings[i]);
}