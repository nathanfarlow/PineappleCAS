#ifndef CAS_H_
#define CAS_H_

#include "../ast.h"

#define is_type_communative(type) (type == OP_ADD || type == OP_MULT)
#define is_type_operator(type) (type >= OP_ADD && type <= OP_LOG)
#define is_type_function(type) (type >= OP_INT && type <= OP_TANH_INV)

void derivative(ast_t *e);

bool simplify(ast_t *e);

double eval(ast_t *e, error *err);

#endif