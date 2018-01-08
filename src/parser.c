#include "parser.h"

#include "stack.h"

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
    /*
    unsigned i;
    for(i = 0; i < t->amount; i++) {
        if(t->tokens[i].type == TI_NUMBER)
            num_Cleanup(t->tokens[i].op.number);
    }
    */
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

/*
Determine if we should insert a multiply operator for a symbol/number with the next token.
For example: 5(2 + 3) and 5x
*/
bool should_multiply_by_next_token(tokenizer_t *tokenizer, unsigned index) {
    token_t current = tokenizer->tokens[index];

    if(index + 1 < tokenizer->amount) {
        token_t next = tokenizer->tokens[index + 1];

                /*5-3*/
        return  next.type == TI_NEGATE
                /*5int(2.5)*/
            ||  (next.type >= TI_INT && next.type <= TI_TANH_INV)
                /*5(2 + x)*/
            ||  next.type == TI_OPEN_PAR
                /*5x or x5*/
            ||  ((next.type == TI_NUMBER || next.type == TI_SYMBOL) && next.type != current.type);

    }

    return false;
}

#define is_type_unary_operator(type)    (type >= TI_NEGATE && type <= TI_CUBE)
#define is_type_binary_operator(type)   (type >= TI_PLUS && type <= TI_ROOT)

#define is_type_unary_function(type)    (type >= TI_INT && type <= TI_TANH_INV)
#define is_type_nnary_function(type)    (type == TI_LOG_BASE)
#define is_type_function(type)          (is_type_unary_function(type) || is_type_nnary_function(type))

uint8_t precedence(TokenType type) {
    switch(type) {
    case TI_PLUS: case TI_MINUS:
        return 5;
    case TI_MULTIPLY: case TI_NEGATE:
    case TI_DIVIDE: case TI_FRACTION:
        return 10;
    case TI_POWER: case TI_RECIPROCAL:
    case TI_SQUARE: case TI_CUBE:
    case TI_ROOT:
        return 15;
    case TI_SCIENTIFIC:
        return 20;
    default:
        return 0;
    }
}

uint8_t operand_count(TokenType type) {
    if(type >= TI_INT && type <= TI_TANH_INV)
        return 1;
    if((type >= TI_PLUS && type <= TI_ROOT) || type == TI_LOG_BASE)
        return 2;
    return 0;
}

bool collapse_precedence(stack_t *operators, stack_t *expressions, TokenType type) {

    while(operators->top > 0) {
        ast_t *collapsed;
        token_t *op;
        unsigned i;

        /*Break when we meet the corresponding (*/
        if(type == TI_CLOSE_PAR && ((token_t*)stack_Peek(operators))->type == TI_OPEN_PAR)
            break;
        /*Break when we meet the function that the , belongs to*/
        else if(type == TI_COMMA && is_type_nnary_function(((token_t*)stack_Peek(operators))->type))
            break;
        /*Break if the precedence is lower than the type*/
        else if(type != TI_CLOSE_PAR && type != TI_COMMA && precedence(((token_t*)stack_Peek(operators))->type) < precedence(type))
            break;

        op = stack_Pop(operators);

        collapsed = ast_MakeOperator(op->type);
        for(i = 0; i < operand_count(op->type); i++) {
            ast_t *operand = stack_Pop(expressions);

            if(operand == NULL)
                return false;

            ast_ChildInsert(collapsed, operand, 0);
        }

        stack_Push(expressions, collapsed);

    }

    return true;
}

bool collapse_all(stack_t *operators, stack_t *expressions) {
    return collapse_precedence(operators, expressions, 0);
}

#define parse_assert(expression, err) if(!(expression)) {stack_Cleanup(&operators); stack_Cleanup(&expressions); *e = err; return NULL;}

ast_t *parse(const uint8_t *equation, unsigned length, error *e) {
    tokenizer_t tokenizer = {0};
    stack_t operators, expressions;
    ast_t *root;

    /*Create instances to push on the stacks as pointers*/
    token_t mult =      {TI_MULTIPLY};
    token_t open_par =  {TI_OPEN_PAR};

    unsigned i;

    *e = tokenize(&tokenizer, equation, length);

    if(*e != E_SUCCESS)
        return NULL;

    stack_Create(&operators);
    stack_Create(&expressions);

    for(i = 0; i < tokenizer.amount; i++) {
        token_t *tok = &tokenizer.tokens[i];

        if(tok->type == TI_OPEN_PAR) {
            stack_Push(&operators, tok);
        } else if(tok->type == TI_NUMBER || tok->type == TI_SYMBOL) {

            if(tok->type == TI_NUMBER) {
                stack_Push(&expressions, ast_MakeNumber(tok->op.number));
            } else {
                stack_Push(&expressions, ast_MakeSymbol(tok->op.symbol));
            }

            if(should_multiply_by_next_token(&tokenizer, i)) {
                parse_assert(collapse_precedence(&operators, &expressions, TI_MULTIPLY), E_PARSE_BAD_OPERATOR);
                stack_Push(&operators, &mult);
            }

        } else if(is_type_unary_operator(tok->type)) {
            stack_Push(&operators, tok);

            if(should_multiply_by_next_token(&tokenizer, i)) {
                parse_assert(collapse_precedence(&operators, &expressions, TI_MULTIPLY), E_PARSE_BAD_OPERATOR);
                stack_Push(&operators, &mult);
            }
        } else if(is_type_binary_operator(tok->type)) {
            parse_assert(collapse_precedence(&operators, &expressions, tok->type), E_PARSE_BAD_OPERATOR);
            stack_Push(&operators, tok);
        } else if(is_type_function(tok->type)) {
            /*Insert a ( to correspond with the other closing ) following the parameters*/
            stack_Push(&operators, &open_par);
            stack_Push(&operators, tok);
        } else if(tok->type == TI_CLOSE_PAR) {
            parse_assert(collapse_precedence(&operators, &expressions, TI_CLOSE_PAR), E_PARSE_BAD_OPERATOR);

            parse_assert(operators.top > 0 && ((token_t*)stack_Peek(&operators))->type == TI_OPEN_PAR, E_PARSE_UNMATCHED_CLOSE_PAR);
            stack_Pop(&operators);

            if(should_multiply_by_next_token(&tokenizer, i)) {
                parse_assert(collapse_precedence(&operators, &expressions, TI_MULTIPLY), E_PARSE_BAD_OPERATOR);
                stack_Push(&operators, &mult);
            }
        } else if(tok->type == TI_COMMA) {
            parse_assert(collapse_precedence(&operators, &expressions, TI_COMMA), E_PARSE_BAD_OPERATOR);
            parse_assert(operators.top > 0 && is_type_function(((token_t*)stack_Peek(&operators))->type), E_PARSE_BAD_COMMA);
        }
    }

    parse_assert(collapse_all(&operators, &expressions), E_PARSE_BAD_OPERATOR);

    root = stack_Pop(&expressions);

    stack_Cleanup(&operators);
    stack_Cleanup(&expressions);

    tokenizer_Cleanup(&tokenizer);

    return root;
}