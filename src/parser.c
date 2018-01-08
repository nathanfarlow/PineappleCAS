#include "parser.h"

struct Identifier token_table[AMOUNT_TOKENS] = {
    {0, {0}}, {0, {0}},         /*TI_NUMBER, TI_SYMBOL*/

    {1, {0x70 }}, {1, {0x71 }}, /*TI_PLUS, TI_MINUS*/
    {1, {0x82 }}, {1, {0x83 }}, /*TI_MULTIPLY, TI_DIVIDES*/
    {2, {0xEF, 0x2E}},          /*TI_FRACTION*/
    {1, {0xF0}},                /*TI_POWER*/
    {1, {0x3B}},                /*TI_SCIENTIFIC*/
    {1, {0xF1}},                /*TI_ROOT*/

    {1, {0xB0}},                            /*TI_NEGATE*/
    {1, {0x0C}}, {1, {0x0D}}, {1, {0x0F}},  /*TI_RECIPROCAL, TI_SQUARE, TI_CUBE*/

    {2, {0xEF, 0x34}},          /*TI_LOG_BASE*/

    {1, {0xB1}}, {1, {0xB2}},   /*TI_INT, TI_ABS*/
    {1, {0xBC}}, {1, {0xBD}},   /*TI_SQRT, TI_CUBED_ROOT*/
    {1, {0xBE}}, {1, {0xBF}},   /*TI_LN, TI_E_TO_POWER*/
    {1, {0xC0}}, {1, {0xC1}},   /*TI_LOG, TI_10_TO_POWER*/
    {1, {0xC2}}, {1, {0xC3}},   /*TI_SIN, TI_SIN_INV*/
    {1, {0xC4}}, {1, {0xC5}},   /*TI_COS, TI_COS_INV*/
    {1, {0xC6}}, {1, {0xC7}},   /*TI_TAN, TI_TAN_INV*/
    {1, {0xC8}}, {1, {0xC9}},   /*TI_SINH, TI_SINH_INV*/
    {1, {0xCA}}, {1, {0xCB}},   /*TI_COSH, TI_COSH_INV*/
    {1, {0xCC}}, {1, {0xCD}},   /*TI_TANH, TI_TANH_INV*/

    {1, {0x10}}, {1, {0x11}},   /*TI_OPEN_PAR, TI_CLOSE_PAR*/
    {1, {0x2B}}, {1, {0x3A}},   /*TI_COMMA, TI_PERIOD*/

    {2, {0xBB, 0x31}},          /*TI_E*/
    {1, {0xAC}}, {1, {0x5B}}    /*TI_PI, TI_THETA*/
};

typedef struct _Token {
    TokenType type;

    union {
        num_t *number;
        Symbol symbol;
    } op;

} token_t;

typedef struct _Tokenizer {
    unsigned amount;
    token_t *tokens;
} tokenizer_t;

void tokenizer_Cleanup(tokenizer_t *t) {
    unsigned i;
    for(i = 0; i < t->amount; i++) {
        if(t->tokens[i].type == TI_NUMBER)
            num_Cleanup(t->tokens[i].op.number);
    }
    free(t->tokens);
}

/*'0' through '9' and including '.'*/
#define is_num(byte) ((byte >= 0x30 && byte <= 0x39) || byte == token_table[TI_PERIOD].bytes[0])

num_t *read_num(const uint8_t *equation, unsigned index, unsigned length, unsigned *consumed) {
    num_t *num;

    unsigned size = 0;
    unsigned i;

    for(i = index; i < length; i++) {
        if(is_num(equation[i])) size++;
        else break;
    }

    num = malloc(sizeof(num_t));

    num->length = size;
    num->digits = malloc(num->length);

    /*Copy digits, but replace Ti's '.' with ascii '.'*/
    for (i = 0; i < size; i++) {
        num->digits[i] = equation[i + index] == token_table[TI_PERIOD].bytes[0] ? '.' : equation[i + index];
    }

    *consumed = size;

    return num;
}

TokenType read_type(const uint8_t *equation, unsigned index, unsigned length, unsigned *consumed) {
    unsigned identifier_index;

    for(identifier_index = TI_PLUS; identifier_index < AMOUNT_TOKENS; identifier_index++) {
        struct Identifier current = token_table[identifier_index];
        bool matches = true;
        unsigned i;

        /*If there is not enough bytes left in the equation for this multi-byte token*/
        if(length - index < current.length)
            continue;

        /*Check if each byte matches*/
        for(i = index; i < index + current.length; i++) {
            matches &= equation[i] == current.bytes[i - index];
            if(!matches) break;
        }

        if(matches) {
            *consumed = current.length;
            return identifier_index;
        }
    }

    return TI_INVALID;
}

Symbol read_symbol(const uint8_t *equation, unsigned index, unsigned length, unsigned *consumed) {

    /*Symbol letter enum values are mapped to their ascii code (SYM_B == 'B')*/
    if(equation[index] >= 'A' && equation[index] <= 'Z') {
        *consumed = 1;
        return equation[index];
    }

    switch(read_type(equation, index, length, consumed)) {
    case TI_EULER:  return SYM_EULER;
    case TI_PI:     return SYM_PI;
    case TI_THETA:  return SYM_THETA;
    default:        return SYM_INVALID;
    }

}

token_t read_token(const uint8_t *equation, unsigned index, unsigned length, unsigned *consumed) {
    token_t tok;

    if(is_num(equation[index])) {

        tok.type = TI_NUMBER;
        tok.op.number = read_num(equation, index, length, consumed);

    } else if(read_symbol(equation, index, length, consumed) != SYM_INVALID) {

        tok.type = TI_SYMBOL;
        tok.op.symbol = read_symbol(equation, index, length, consumed);

    } else {
        tok.type = read_type(equation, index, length, consumed);

        /*Skip over invalid bytes if choose to not catch error*/
        if(tok.type == TI_INVALID)
            *consumed = 1;
    }

    return tok;
}

error _tokenize(token_t *tokens, const uint8_t *equation, unsigned length, unsigned *tok_amount) {
    unsigned token_index = 0;
    unsigned i = 0;

    while(i < length) {
        token_t tok;
        unsigned consumed;

        tok = read_token(equation, i, length, &consumed);

        if(tok.type == TI_INVALID)
            return E_TOK_INVALID;

        if(tokens != NULL) {
            tokens[token_index] = tok;
        } else if(tok.type == TI_NUMBER) {
            /*Clean up the number if it's not being saved*/
            num_Cleanup(tok.op.number);
        }

        token_index++;
        i += consumed;
    }

    *tok_amount = token_index;

    return E_SUCCESS;
}

error tokenize(tokenizer_t *t, const uint8_t *equation, unsigned length) {
    error err;

    /*Determine the amount of tokens to malloc()*/
    err = _tokenize(NULL, equation, length, &t->amount);

    if(err != E_SUCCESS)
        return err;

    /*Malloc and fill tokens*/
    t->tokens = malloc(t->amount * sizeof(token_t));
    err = _tokenize(t->tokens, equation, length, &t->amount);

    return err;
}

ast_t *parse(const uint8_t *equation, unsigned length, error *e) {
    tokenizer_t tokenizer = {0};

    *e = tokenize(&tokenizer, equation, length);

    tokenizer_Cleanup(&tokenizer);

    return NULL;
}