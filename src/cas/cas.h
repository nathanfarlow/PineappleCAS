#ifndef CAS_H_
#define CAS_H_

#include "../ast.h"

ast_t *derivative(ast_t *e);
ast_t *simplify(ast_t *e);

#endif