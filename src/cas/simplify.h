#ifndef SIMPLIFY_H_
#define SIMPLIFY_H_

#include "cas.h"

bool simplify_commutative(ast_t *e);
bool simplify_rational(ast_t *e);
bool simplify_normalize(ast_t *e);
bool eval(ast_t *e);
bool simplify_canonical_form(ast_t *e);
bool simplify_like_terms(ast_t *e);


bool is_negative_for_sure(ast_t *a);
/*Returns true if changed. Expects completely simplified.*/
bool absolute_val(ast_t *e);

#endif