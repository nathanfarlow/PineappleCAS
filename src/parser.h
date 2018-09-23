#ifndef PARSER_H_
#define PARSER_H_

#include "num.h"
#include "ast.h"

#include "error.h"

/*Ti Tokens, used for reading from yvars*/
typedef enum _TokenType {
    TI_NUMBER, TI_SYMBOL, /*numbers, variables, pi, e*/

    TI_PLUS, TI_MINUS,
    TI_MULTIPLY, TI_DIVIDE,
    TI_FRACTION, /*special n/d for ti pretty print*/
    TI_PROPER,   /*special Un/d for ti pretty print*/
    TI_POWER,
    TI_SCIENTIFIC,
    TI_ROOT,

    /*Unary*/
    TI_NEGATE,
    TI_RECIPROCAL, TI_SQUARE, TI_CUBE,
    TI_FACTORIAL,

    TI_LOG_BASE,

    TI_INT, TI_ABS,
    
    TI_SQRT, TI_CUBED_ROOT,

    TI_LN, TI_E_TO_POWER,
    TI_LOG, TI_10_TO_POWER,

    TI_SIN, TI_SIN_INV,
    TI_COS, TI_COS_INV,
    TI_TAN, TI_TAN_INV,
    TI_SINH, TI_SINH_INV,
    TI_COSH, TI_COSH_INV,
    TI_TANH, TI_TANH_INV,

    TI_OPEN_PAR, TI_CLOSE_PAR, TI_COMMA, TI_PERIOD,

    TI_EULER, TI_PI, TI_THETA,

    AMOUNT_TOKENS, TI_INVALID
} TokenType;

struct Identifier {
    uint8_t length;
    uint8_t bytes[2];
};

extern struct Identifier token_table[AMOUNT_TOKENS];

ast_t *parse(const uint8_t *equation, unsigned length, error_t *e);

#endif