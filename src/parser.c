#include "parser.h"

struct Identifier table[AMOUNT_TOKENS] = {
    {0, {0}}, {0, {0}},         //TI_NUMBER, TI_SYMBOL

    {1, {0x70 }}, {1, {0x71 }}, //TI_PLUS, TI_MINUS
    {1, {0x82 }}, {1, {0x83 }}, //TI_MULTIPLY, TI_DIVIDES
    {2, {0xEF, 0x2E}},          //TI_FRACTION
    {1, {0xF0}},                //TI_POWER
    {1, {0x3B}},                //TI_SCIENTIFIC
    {1, {0xF1}},                //TI_ROOT

    {1, {0xB0}},                            //TI_NEGATE
    {1, {0x0C}}, {1, {0x0D}}, {1, {0x0F}},  //TI_RECIPROCAL, TI_SQUARE, TI_CUBE

    {2, {0xEF, 0x34}},          //TI_LOG_BASE

    {1, {0xB1}}, {1, {0xB2}},   //TI_INT, TI_ABS
    {1, {0xBC}}, {1, {0xBD}},   //TI_SQRT, TI_CUBED_ROOT
    {1, {0xBE}}, {1, {0xBF}},   //TI_LN, TI_E_TO_POWER
    {1, {0xC0}}, {1, {0xC1}},   //TI_LOG, TI_10_TO_POWER
    {1, {0xC2}}, {1, {0xC3}},   //TI_SIN, TI_SIN_INV
    {1, {0xC4}}, {1, {0xC5}},   //TI_COS, TI_COS_INV
    {1, {0xC6}}, {1, {0xC7}},   //TI_TAN, TI_TAN_INV
    {1, {0xC8}}, {1, {0xC9}},   //TI_SINH, TI_SINH_INV
    {1, {0xCA}}, {1, {0xCB}},   //TI_COSH, TI_COSH_INV
    {1, {0xCC}}, {1, {0xCD}},   //TI_TANH, TI_TANH_INV

    {1, {0x10}}, {1, {0x11}},   //TI_OPEN_PAR, TI_CLOSE_PAR,
    {1, {0x2B}}, {1, {0x3A}}    //TI_COMMA, TI_PERIOD
};

typedef struct _Token {
    TokenType type;

    union {
        num_t number;
        uint8_t symbol;
    } op;

} token_t;

typedef struct _Tokenizer {
    unsigned amount;
    token_t *tokens;
} tokenizer_t;

error tokenize(tokenizer_t *t, const uint8_t *equation, unsigned length) {
    return E_SUCCESS;
}

ast_t *parse(const uint8_t *equation, unsigned length, error *e) {
    tokenizer_t tokenizer;
    *e = tokenize(&tokenizer, equation, length);

    return NULL;
}