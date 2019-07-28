#ifndef CAS_H_
#define CAS_H_

#include "../ast.h"
#include "derivative.h"

/*
    Changes ast to a form we work with in the simpilfier.

    Reduces every number node to numerator integer / denominator integer
    Changes all roots to powers.
    Expands (AB)^5 to A^5 * B^5

    Generally speaking, this should be included in every simplify() call
*/
#define SIMP_NORMALIZE                  (1 << 0)
/*
    Flattens multiplication and addition nodes
    Also changes a multiplication or addition node with only one child
    into just that one child.
*/
#define SIMP_COMMUTATIVE                (1 << 1)
/*
    Simplifies rational functions.
    A/B/C/D becomes A/(BCD)
*/
#define SIMP_RATIONAL                   (1 << 2)
/*
    Simply executes eval() on the node which will evaluate all constant
    expressions like 5 + 5 to 10.
*/
#define SIMP_EVAL                       (1 << 3)
/*
    Change A + A to 2A
    Change A*A to A^2
    Change A_A to 0
*/
#define SIMP_LIKE_TERMS                 (1 << 4)
/*
    Simplifies inverses like sin(asin(X)) = X
    as well as log identities
*/
#define SIMP_ID_GENERAL                 (1 << 5)
/*
    Simplifies trig identities like sin(X)^2 + cos(X)^2 = 1

    ANY SIMP_ID WILL AUTOMATICALLY SET THE EVAL FLAG, COMMUTATIVE FLAG, AND THE SIMPLIFY LIKE TERMS FLAG
    ANY SIMP_ID WILL ALSO CALL TO EXPAND() AND FACTOR()
*/
#define SIMP_ID_TRIG                    (1 << 6)
/*
    Simplifies trig constants like sin(pi/4) = sqrt(2)/2

    ANY SIMP_ID WILL AUTOMATICALLY SET THE EVAL FLAG, COMMUTATIVE FLAG, AND THE SIMPLIFY LIKE TERMS FLAG
    ANY SIMP_ID WILL ALSO CALL TO EXPAND() AND FACTOR()
*/
#define SIMP_ID_TRIG_CONSTANTS          (1 << 7)
/*
    Simplifies hyperbolic identities like sinh(X)/cosh(X) = tanh(X)

    ANY SIMP_ID WILL AUTOMATICALLY SET THE EVAL FLAG, COMMUTATIVE FLAG, AND THE SIMPLIFY LIKE TERMS FLAG
    ANY SIMP_ID WILL ALSO CALL TO EXPAND() AND FACTOR()
*/
#define SIMP_ID_HYPERBOLIC              (1 << 8)
/*
    Simplifies complex functins like sin(z) and ln(z)

    ANY SIMP_ID WILL AUTOMATICALLY SET THE EVAL FLAG, COMMUTATIVE FLAG, AND THE SIMPLIFY LIKE TERMS FLAG
    ANY SIMP_ID WILL ALSO CALL TO EXPAND() AND FACTOR()
*/
#define SIMP_ID_COMPLEX                 (1 << 9)
/*
    Simplify all identities

    ANY SIMP_ID WILL AUTOMATICALLY SET THE EVAL FLAG, COMMUTATIVE FLAG, AND THE SIMPLIFY LIKE TERMS FLAG
    ANY SIMP_ID WILL ALSO CALL TO EXPAND() AND FACTOR()
*/
#define SIMP_ID_ALL                     (SIMP_ID_GENERAL | SIMP_ID_TRIG | SIMP_ID_TRIG_CONSTANTS | SIMP_ID_HYPERBOLIC)

#define SIMP_ALL                        (0xFFFF)

/*Simplifies ast. Returns true if changed*/
bool simplify(pcas_ast_t *e, unsigned short flags);

/*
    Order multiplication and division.
    Change XAZ to AXZ and 1+sin(X)ZA5 to 5AZsin(X)+1
    Rationalize denominators.
    Change powers to roots.
    Combine a^5b^5 to (ab)^5.

    Use only when exporting and do not plan to simplify again
    or execute any other function on ast. Messes up all other algorithms.

    Sorting for addition and multiplication is O(n^2) by insertion sort
*/
bool simplify_canonical_form(pcas_ast_t *e);

/*Factor A + A to A(1 + 1) where at least one part of the
resulting multiplication can be evauated numerically*/
#define FAC_SIMPLE_ADDITION_EVALUATEABLE    (1 << 0)
/*Factor A^3+AX to A(A^2+X)*/
#define FAC_SIMPLE_ADDITION_NONEVALUATEABLE (1 << 1)
/*Factor  x^2 - 1 to (x-1)(x+1)*/
#define FAC_POLYNOMIAL                      (1 << 2)
#define FAC_ALL                             0xFF

bool factor(pcas_ast_t *e, unsigned char flags);

/*EVAL_DISTRIBUTE_NUMBERS, EVAL_DISTRIBUTE_MULTIPLICATION, EVAL_DISTRIBUTE_ADDITION, EVAL_EXPAND_POWERS*/

/*Expand 2(A+B) to 2A + 2B or -(A+B) to -1*A + -1*B*/
#define EXP_DISTRIB_NUMBERS        (1 << 0)
/*Expand any f(x)*g(x)*(A+B) to  f(x)g(x)A + f(x)g(x)B*/
#define EXP_DISTRIB_MULTIPLICATION (1 << 1)
/*Expand (AB)/2 to (1/2)AB and (A+B)/2 to 1/2A + 1/2B*/
#define EXP_DISTRIB_DIVISION       (1 << 2)
/*Expand (A+B)(C+D) to AC+AD+BC+BD */
#define EXP_DISTRIB_ADDITION       (1 << 3)
/*Expand (A+B)^2 to A^2 + 2AB + B^2 */
#define EXP_EXPAND_POWERS          (1 << 4)
#define EXP_ALL                    0xFF

bool expand(pcas_ast_t *e, unsigned char flags);

/*Evaluates identities that are too basic to put in identities.c*/
#define EVAL_BASIC_IDENTITIES   (1 << 0)
/*Evaluates addition and multiplication of constants*/
#define EVAL_COMMUTATIVE        (1 << 1)
/*Evaluates 4/2 to 2 and 15/20 to 3/4*/
#define EVAL_DIVISION           (1 << 2)
/*Evaluates a^b if a <= 10 and b <= 10*/
#define EVAL_POWERS_SMALL       (1 << 3)
#define EVAL_POWERS_FULL        (1 << 4)
/*Evaluates int(5.5) = 5*/
#define EVAL_INT                (1 << 5)
/*Evaluates absolute value of strictly real constants*/
#define EVAL_ABS                (1 << 6)
/*Evaluates X! where X <= 10*/
#define EVAL_FACTORIAL_SMALL    (1 << 7)
#define EVAL_FACTORIAL_FULL     (1 << 8)
/*Things that can be computed in a reasonable amount of time*/
#define EVAL_EASY               (EVAL_BASIC_IDENTITIES | EVAL_COMMUTATIVE | EVAL_DIVISION | EVAL_INT | EVAL_ABS | EVAL_POWERS_SMALL | EVAL_FACTORIAL_SMALL)
/*Things that take a lot of computation to compute*/
#define EVAL_HARD               (EVAL_POWERS_FULL | EVAL_FACTORIAL_FULL)
#define EVAL_ALL                0xFFFF

/*Evaluates constants such as 5+5 or 6^5. Also implements basic identities
such as 1A = A, A + 0 = A. Returns true if the ast was changed.*/
bool eval(pcas_ast_t *e, unsigned short flags);

/*Replaces all instances of from ast to to ast in e*/
bool substitute(pcas_ast_t *e, pcas_ast_t *from, pcas_ast_t *to);

/*Returns the greatest common divisor of two expressions.
gcd(X(X+1)^2, AX) = X 
gcd(6AX, 10X) = 2X*/
pcas_ast_t *gcd(pcas_ast_t *a, pcas_ast_t *b);

/*Helper functions*/

/*Cleanup a. Copy a to b and cleanup b.*/
void replace_node(pcas_ast_t *a, pcas_ast_t *b);

/*Returns true if the node has an imaginary node.*/
bool has_imaginary_node(pcas_ast_t *e);

/*Returns true if the node is being multiplied by at least one negative or is already a negative number node.
The node must be completely simplified for this to work, because it does not detect
multiplying by more than one negative to make a positive. */
bool is_negative_for_sure(pcas_ast_t *a);

/*Returns true if changed. Expects completely simplified. Removes the negative in the multiplier or number.*/
bool absolute_val(pcas_ast_t *e);

#endif