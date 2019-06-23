#ifndef CAS_H_
#define CAS_H_

#include "../ast.h"

/*Evaluates constants. Returns true if changed*/
bool eval(ast_t *e);

ast_t *gcd(ast_t *a, ast_t *b);

/*Factors simple addition. Does not factor polynomials.*/
void factor_addition(ast_t *e);
/*Simplifies ast. Returns true if changed*/
bool simplify(ast_t *e);

void derivative(ast_t *e);

/*Helper functions*/

/*Change a to b and delete b*/
void replace_node(ast_t *a, ast_t *b);

#endif