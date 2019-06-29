#ifndef CAS_H_
#define CAS_H_

#include "../ast.h"

/*
    Changes ast to a form we work with in the simpilfier.

    Reduces every number node to numerator integer / denominator integer
    Changes all roots to powers.
    Expands (AB)^5 to A^5 * B^5

    Generally speaking, this should be included in every simplify() call
*/
#define SIMP_NORMALIZE      (1 << 0)
/*
    Flattens multiplication and addition nodes
    Also changes a multiplication or addition node with only one child
    into just that one child.
*/
#define SIMP_COMMUTATIVE    (1 << 1)
/*
    Simplifies rational functions.
    A/B/C/D becomes A/(BCD)
*/
#define SIMP_RATIONAL       (1 << 2)
/*
    Simply executes eval() on the node which will evaluate all constant
    expressions like 5 + 5 to 10.
*/
#define SIMP_EVAL           (1 << 3)
/*
    Order multiplication and division.
    Change XAZ to AXZ and 1+sin(X)ZA5 to 5AZsin(X)+1
    Sorting for addition and multiplication is O(n^2) by insertion sort
*/
#define SIMP_CANONICAL_FORM (1 << 4)
/*
    Change A + A to 2A
    Change A*A to A^2
    Change A_A to 0
*/
#define SIMP_LIKE_TERMS     (1 << 5)
/*
    Simplifies from a dictionary of identities.
    Ex. sin(ln(X))^2 + cos(ln(X))^2 = 1.
*/
#define SIMP_IDENTITIES     (1 << 6)
#define SIMP_ALL            0xFF

/*Simplifies ast. Returns true if changed*/
bool simplify(ast_t *e, const unsigned char flags);

/*Factor A + A to A(1 + 1) where at least one part of the
resulting multiplication can be evauated numerically*/
#define FAC_SIMPLE_ADDITION_EVALUATEABLE    (1 << 0)
/*Factor A^3+AX to A(A^2+X)*/
#define FAC_SIMPLE_ADDITION_NONEVALUATEABLE (1 << 1)
/*Factor  x^2 - 1 to (x-1)(x+1)*/
#define FAC_POLYNOMIAL                      (1 << 2)
#define FAC_ALL                             0xFF

bool factor(ast_t *e, const unsigned char flags);

/*EVAL_DISTRIBUTE_NUMBERS, EVAL_DISTRIBUTE_MULTIPLICATION, EVAL_DISTRIBUTE_ADDITION, EVAL_EXPAND_POWERS*/

/*Expand 2(A+B) to 2A + 2B or -(A+B) to -1*A + -1*B*/
#define EXP_DISTRIB_NUMBERS        (1 << 0)
/*Expand any f(x)*g(x)*(A+B) to  f(x)g(x)A + f(x)g(x)B*/
#define EXP_DISTRIB_MULTIPLICATION (1 << 1)
/*Expand (A+B)(C+D) to AC+AD+BC+BD */
#define EXP_DISTRIB_ADDITION       (1 << 2)
/*Expand (A+B)^2 to A^2 + 2AB + B^2 */
#define EXP_EXPAND_POWERS          (1 << 3)
#define EXP_ALL                    0xFF

bool expand(ast_t *e, const unsigned char flags);


/*Evaluates constants such as 5+5 or 6^5. Also implements basic identities
such as 1A = A, A + 0 = A. Returns true if the ast was changed.*/
bool eval(ast_t *e);

/*Returns the greatest common divisor of two expressions.
gcd(X(X+1)^2, AX) = X 
gcd(6AX, 10X) = 2X*/
ast_t *gcd(ast_t *a, ast_t *b);

void derivative(ast_t *e);

/*Helper functions*/

/*Cleanup a. Copy a to b and cleanup b.*/
void replace_node(ast_t *a, ast_t *b);

/*Returns true if the node is being multiplied by at least one negative or is already a negative number node.
The node must be completely simplified for this to work, because it does not detect
multiplying by more than one negative to make a positive. */
bool is_negative_for_sure(ast_t *a);

/*Returns true if changed. Expects completely simplified. Removes the negative in the multiplier or number.*/
bool absolute_val(ast_t *e);

#endif