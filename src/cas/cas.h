#ifndef CAS_H_
#define CAS_H_

#include "../ast.h"

/*Evaluates constants. Returns true if changed*/
bool eval(ast_t *e);

ast_t *gcd(ast_t *a, ast_t *b);

/*Factors simple addition. Does not factor polynomials.
if eval_only is set, then it will only factor Things like A + 2A where one
of the multiplications nodes can be evaluated to a number, like A(1 + 2)
*/
void factor_addition(ast_t *e, bool eval_only);
/*Simplifies ast. Returns true if changed*/
bool simplify(ast_t *e);

void derivative(ast_t *e);

/*Expands (5X + 3)(2X+5) or (3+D)^5*/
bool expand(ast_t *e, bool expand_powers);

/*Helper functions*/

/*Change a to b and delete b*/
void replace_node(ast_t *a, ast_t *b);

#endif