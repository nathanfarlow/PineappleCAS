#ifndef PARSER_H_
#define PARSER_H_

#include "ast.h"
#include "error.h"

#define is_tok_unary_operator(tok)    (tok >= TOK_NEGATE && tok <= TOK_FACTORIAL)
#define is_tok_binary_operator(tok)   (tok >= TOK_PLUS && tok <= TOK_ROOT)
#define is_tok_operator(tok) (is_tok_unary_operator(tok) || is_tok_binary_operator(tok))

#define is_tok_unary_function(tok)    (tok >= TOK_INT && tok <= TOK_TANH_INV)
#define is_tok_nary_function(tok)     (tok == TOK_LOG_BASE)
#define is_tok_function(tok)          (is_tok_unary_function(tok) || is_tok_nary_function(tok))

/*Ti Tokens, used for reading from yvars*/
typedef enum _TokenType {
    TOK_NUMBER, TOK_SYMBOL, /*numbers, variables, pi, e*/

    TOK_PLUS, TOK_MINUS,
    TOK_MULTIPLY, TOK_DIVIDE,
    TOK_FRACTION, /*special n/d for ti pretty print*/
    TOK_PROPER,   /*special Un/d for ti pretty print*/
    TOK_POWER,
    TOK_SCIENTIFIC,
    TOK_ROOT,

    /*Unary*/
    TOK_NEGATE,
    TOK_RECIPROCAL, TOK_SQUARE, TOK_CUBE,
    TOK_FACTORIAL,

    TOK_LOG_BASE,

    TOK_INT, TOK_ABS,
    
    TOK_SQRT, TOK_CUBED_ROOT,

    TOK_LN, TOK_E_TO_POWER,
    TOK_LOG, TOK_10_TO_POWER,

    TOK_SIN, TOK_SIN_INV,
    TOK_COS, TOK_COS_INV,
    TOK_TAN, TOK_TAN_INV,
    TOK_SINH, TOK_SINH_INV,
    TOK_COSH, TOK_COSH_INV,
    TOK_TANH, TOK_TANH_INV,

    TOK_OPEN_PAR, TOK_CLOSE_PAR, TOK_COMMA, TOK_PERIOD,

    TOK_IMAG,

    TOK_EULER, TOK_PI, TOK_THETA,

    AMOUNT_TOKENS, TOK_INVALID
} TokenType;

#define MAX_IDENTIFIER_LEN 7

struct Identifier {
    uint8_t length;
    uint8_t bytes[MAX_IDENTIFIER_LEN];
};

extern struct Identifier ti_table[AMOUNT_TOKENS];
extern struct Identifier str_table[AMOUNT_TOKENS];

ast_t *parse(const uint8_t *equation, unsigned length, struct Identifier *lookup, error_t *e);
uint8_t *export_to_binary(ast_t *e, unsigned *len, struct Identifier *lookup, error_t *err);

#endif