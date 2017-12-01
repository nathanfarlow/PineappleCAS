/*
	PineappleCAS: the multi-purpose CAS specifically for the TI-84+ CE

	Authors:
	Nathan Farlow
*/

#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

void print_ast(ast_t *e) {
	ast_t *current;
	for(current = e->op.operator.base; current != NULL; current = current->next) {
		printf("%c\n", current->op.symbol);
	}
}

int main(int argc, char **argv) {

	ast_t *parent = ast_MakeOperator(2);
	char i;
	for(i = 'a'; i < 'h'; i++) {
		ast_ChildAppend(parent, ast_MakeSymbol(i));
	}

	
	printf("%i\n", ast_ChildLength(parent));

	print_ast(parent);

	ast_Cleanup(parent);
	return 0;
}