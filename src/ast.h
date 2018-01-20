#ifndef AST_H_
#define AST_H_

#include "num.h"
#include "error.h"

#include <stdlib.h> /*For malloc*/

#define LSIZE unsigned

typedef enum {
    NODE_NUMBER, NODE_SYMBOL, NODE_OPERATOR
} NodeType;

typedef enum {
    OP_ADD, /*OP_SUB,*/
    OP_MULT, OP_DIV,

    OP_POW, OP_ROOT,
    OP_LOG,

    OP_INT, OP_ABS,

    OP_SIN, OP_SIN_INV,
    OP_COS, OP_COS_INV,
    OP_TAN, OP_TAN_INV,

    OP_SINH, OP_SINH_INV,
    OP_COSH, OP_COSH_INV,
    OP_TANH, OP_TANH_INV,

    AMOUNT_OPS
} OperatorType;

#define AMOUNT_SYMBOLS 29

typedef enum {
    SYM_PI, SYM_EULER, SYM_THETA,

    SYM_A = 'A',
    SYM_B, SYM_C, SYM_D, SYM_E, SYM_F,
    SYM_G, SYM_H, SYM_I, SYM_J, SYM_K,
    SYM_L, SYM_M, SYM_N, SYM_O, SYM_P,
    SYM_Q, SYM_R, SYM_S, SYM_T, SYM_U,
    SYM_V, SYM_W, SYM_X, SYM_Y, SYM_Z,

    SYM_INVALID
} Symbol;

typedef struct _Node {

    NodeType type;
    /*For the linked list implementation*/
    struct _Node *next;

    union {
        /*NODE_NUMBER*/
        num_t *number;

        /*NODE_SYMBOL*/
        Symbol symbol;

        /*NODE_OPERATOR*/
        struct {
            OperatorType type;

            /*The base node for the linked list*/
            struct _Node *base;

        } operator;
    } op;

} ast_t;

ast_t *ast_MakeNumber(num_t *num);
ast_t *ast_MakeSymbol(char symbol);

ast_t *ast_MakeOperator(OperatorType type);
ast_t *ast_MakeUnary(OperatorType type, ast_t *operand);
ast_t *ast_MakeBinary(OperatorType type, ast_t *left, ast_t *right);

ast_t *ast_Copy(ast_t *e);
bool ast_Compare(ast_t *a, ast_t *b);

void ast_Cleanup(ast_t *e);

/*Functions dealing with the children of operator asts*/
error ast_ChildAppend(ast_t *parent, ast_t *child);
error ast_ChildInsert(ast_t *parent, ast_t *child, LSIZE index);

ast_t *ast_ChildGet(ast_t *parent, LSIZE index);
ast_t *ast_ChildGetLast(ast_t *parent);

/*returns -1 (unsigned) if not found*/
LSIZE ast_ChildIndexOf(ast_t *parent, ast_t *child);

/*
Returns the removed node, null if none
Currently, removeIndex() is much faster, so please
use that if possible.
*/
ast_t *ast_ChildRemove(ast_t *parent, ast_t *child);
ast_t *ast_ChildRemoveIndex(ast_t *parent, LSIZE index);

LSIZE ast_ChildLength(ast_t *parent);

#endif