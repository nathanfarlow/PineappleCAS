#include "parser.h"

#include "stack.h"

/*http://tibasicdev.wikidot.com/one-byte-tokens*/
struct Identifier ti_table[AMOUNT_TOKENS] = {
    {0, {0}}, {0, {0}},         /*TOK_NUMBER, TOK_SYMBOL*/

    {1, {0x70}}, {1, {0x71}},   /*TOK_PLUS, TOK_MINUS*/
    {1, {0x82}}, {1, {0x83}},   /*TOK_MULTIPLY, TOK_DIVIDE*/
    {2, {0xEF, 0x2E}},          /*TOK_FRACTION*/
    {2, {0xEF, 0x2F}},          /*TOK_PROPER*/
    {1, {0xF0}},                /*TOK_POWER*/
    {1, {0x3B}},                /*TOK_SCIENTIFIC*/
    {1, {0xF1}},                /*TOK_ROOT*/

    {1, {0xB0}},                            /*TOK_NEGATE*/
    {1, {0x0C}}, {1, {0x0D}}, {1, {0x0F}},  /*TOK_RECIPROCAL, TOK_SQUARE, TOK_CUBE*/
    {1, {0x2D}},                            /*TOK_FACTORIAL*/

    {2, {0xEF, 0x34}},          /*TOK_LOG_BASE*/
    {1, {0x25}},                /*TOK_DERIV*/

    {1, {0xB1}}, {1, {0xB2}},   /*TOK_INT, TOK_ABS*/
    {1, {0xBC}}, {1, {0xBD}},   /*TOK_SQRT, TOK_CUBED_ROOT*/
    {1, {0xBE}}, {1, {0xBF}},   /*TOK_LN, TOK_E_TO_POWER*/
    {1, {0xC0}}, {1, {0xC1}},   /*TOK_LOG, TOK_10_TO_POWER*/
    {1, {0xC2}}, {1, {0xC3}},   /*TOK_SIN, TOK_SIN_INV*/
    {1, {0xC4}}, {1, {0xC5}},   /*TOK_COS, TOK_COS_INV*/
    {1, {0xC6}}, {1, {0xC7}},   /*TOK_TAN, TOK_TAN_INV*/
    {1, {0xC8}}, {1, {0xC9}},   /*TOK_SINH, TOK_SINH_INV*/
    {1, {0xCA}}, {1, {0xCB}},   /*TOK_COSH, TOK_COSH_INV*/
    {1, {0xCC}}, {1, {0xCD}},   /*TOK_TANH, TOK_TANH_INV*/

    {1, {0x10}}, {1, {0x11}},   /*TOK_OPEN_PAR, TOK_CLOSE_PAR*/
    {1, {0x2B}}, {1, {0x3A}},   /*TOK_COMMA, TOK_PERIOD*/

    {1, {0x2C}},                /*TOK_IMAG*/

    {2, {0xBB, 0x31}},          /*TOK_E*/
    {1, {0xAC}}, {1, {0x5B}}    /*TOK_PI, TOK_THETA*/
};

struct Identifier str_table[AMOUNT_TOKENS] = {
    {0, {0}}, {0, {0}},             /*TOK_NUMBER, TOK_SYMBOL*/

    {1, "+"}, {1, "_"},             /*TOK_PLUS, TOK_MINUS*/
    {1, "*"}, {1, "/"},             /*TOK_MULTIPLY, TOK_DIVIDE*/

    {1, "/"},                       /*TOK_FRACTION*/
    {1, "+"},                       /*TOK_PROPER*/
    {1, "^"},                       /*TOK_POWER*/
    {3, "[E]"},                     /*TOK_SCIENTIFIC*/
    {4, "root"},                    /*TOK_ROOT*/

    {1, "-"},                               /*TOK_NEGATE*/
    {5, "^(-1)"}, {2, "^2"}, {2, "^3"},     /*TOK_RECIPROCAL, TOK_SQUARE, TOK_CUBE*/
    {1, "!"},                               /*TOK_FACTORIAL*/

    {5, "logb("},                   /*TOK_LOG_BASE*/
    {6, "deriv("},                  /*TOK_DERIV*/

    {4, "int("}, {4, "abs("},       /*TOK_INT, TOK_ABS*/
    {5, "sqrt("}, {7, "cbroot("},   /*TOK_SQRT, TOK_CUBED_ROOT*/
    {3, "ln("}, {3, "e^("},         /*TOK_LN, TOK_E_TO_POWER*/
    {4, "log("}, {4, "10^("},       /*TOK_LOG, TOK_10_TO_POWER*/
    {4, "sin("}, {5, "asin("},      /*TOK_SIN, TOK_SIN_INV*/
    {4, "cos("}, {5, "acos("},      /*TOK_COS, TOK_COS_INV*/
    {4, "tan("}, {5, "atan("},      /*TOK_TAN, TOK_TAN_INV*/
    {5, "sinh("}, {6, "asinh("},    /*TOK_SINH, TOK_SINH_INV*/
    {5, "cosh("}, {6, "acosh("},    /*TOK_SINH, TOK_SINH_INV*/
    {5, "tanh("}, {6, "atanh("},    /*TOK_TANH, TOK_TANH_INV*/

    {1, "("}, {1, ")"},             /*TOK_OPEN_PAR, TOK_CLOSE_PAR*/
    {1, ","}, {1, "."},             /*TOK_COMMA, TOK_PERIOD*/

    {1, "i"},                       /*TOK_IMAG */

    {1, "e"},                       /*TOK_E*/
    {2, "pi"}, {5, "theta"}         /*TOK_PI, TOK_THETA*/

};

typedef struct _Token {
    TokenType type;

    union {
        mp_rat num;
        Symbol symbol;
    } op;

} token_t;

typedef struct _Tokenizer {
    unsigned amount;
    token_t *tokens;
} tokenizer_t;

/*'0' through '9' and including '.'*/
#define is_num(byte) ((byte >= 0x30 && byte <= 0x39) || byte == lookup[TOK_PERIOD].bytes[0])

mp_rat read_num(const uint8_t *equation, unsigned index, unsigned length, struct Identifier *lookup, unsigned *consumed) {
    mp_rat num;

    unsigned size = 0;
    unsigned i;

    char *buffer;

    for(i = index; i < length; i++) {
        if(is_num(equation[i])) size++;
        else break;
    }

    buffer = malloc(size + 1);

    /*Copy digits, but replace Ti's '.' with ascii '.'*/
    for (i = 0; i < size; i++) {

        char digit = equation[i + index];

        if(digit == lookup[TOK_PERIOD].bytes[0])
            buffer[i] = '.';
        else
            buffer[i] = digit;
    }

    buffer[size] = '\0';

    num = num_FromString(buffer);

    free(buffer);

    *consumed = size;

    return num;
}

TokenType read_type(const uint8_t *equation, unsigned index, unsigned length, struct Identifier *lookup, unsigned *consumed) {
    unsigned identifier_index;

    for(identifier_index = TOK_PLUS; identifier_index < AMOUNT_TOKENS; identifier_index++) {
        struct Identifier current = lookup[identifier_index];
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

    return TOK_INVALID;
}

Symbol read_symbol(const uint8_t *equation, unsigned index, unsigned length, struct Identifier *lookup, unsigned *consumed) {

    /*Symbol letter enum values are mapped to their ascii code (SYM_B == 'B')*/
    if(equation[index] >= 'A' && equation[index] <= 'Z') {
        *consumed = 1;
        return equation[index];
    }

    switch(read_type(equation, index, length, lookup, consumed)) {
    case TOK_IMAG:  return SYM_IMAG;
    case TOK_EULER: return SYM_EULER;
    case TOK_PI:    return SYM_PI;
    case TOK_THETA: return SYM_THETA;
    default:        return SYM_INVALID;
    }

}

token_t read_token(const uint8_t *equation, unsigned index, unsigned length, struct Identifier *lookup, unsigned *consumed) {
    token_t tok;

    if(is_num(equation[index])) {

        tok.type = TOK_NUMBER;
        tok.op.num = read_num(equation, index, length, lookup, consumed);

    } else if(read_symbol(equation, index, length, lookup, consumed) != SYM_INVALID) {

        tok.type = TOK_SYMBOL;
        tok.op.symbol = read_symbol(equation, index, length, lookup, consumed);

    } else {
        tok.type = read_type(equation, index, length, lookup, consumed);

        /*Skip over invalid bytes if choose to not catch error*/
        if(tok.type == TOK_INVALID)
            *consumed = 1;
    }

    return tok;
}

error_t _tokenize(token_t *tokens, const uint8_t *equation, unsigned length, unsigned *tok_amount, struct Identifier *lookup) {
    unsigned token_index = 0;
    unsigned i = 0;

    while(i < length) {
        token_t tok;
        unsigned consumed;

        tok = read_token(equation, i, length, lookup, &consumed);

        if(tok.type == TOK_INVALID)
            return E_TOK_INVALID;

        if(tokens != NULL) {
            tokens[token_index] = tok;
        } else if(tok.type == TOK_NUMBER) {
            /*Clean up the number if it's not being saved*/
            num_Cleanup(tok.op.num);
        }

        token_index++;
        i += consumed;
    }

    *tok_amount = token_index;

    return E_SUCCESS;
}

error_t tokenize(tokenizer_t *t, const uint8_t *equation, unsigned length, struct Identifier *lookup) {
    error_t err;

    /*Determine the amount of tokens to malloc()*/
    err = _tokenize(NULL, equation, length, &t->amount, lookup);

    if(err != E_SUCCESS)
        return err;

    /*Malloc and fill tokens*/
    t->tokens = malloc(t->amount * sizeof(token_t));
    err = _tokenize(t->tokens, equation, length, &t->amount, lookup);

    return err;
}

/*Larger = Higher precedence*/
uint8_t precedence(TokenType type) {
    switch(type) {
    case TOK_PLUS: case TOK_MINUS:
        return 5;
    case TOK_MULTIPLY: case TOK_NEGATE:
    case TOK_DIVIDE: case TOK_FRACTION:
        return 10;
    case TOK_POWER:  case TOK_RECIPROCAL:
    case TOK_SQUARE: case TOK_CUBE:
    case TOK_ROOT:   case TOK_FACTORIAL:
        return 15;
    case TOK_SCIENTIFIC:
        return 20;
    default:
        return 0;
    }
}

/*
Determine if we should insert a multiply operator for a symbol/number with the next token.
For example: 5(2 + 3) and 5x
*/
bool should_multiply_by_next_token(tokenizer_t *tokenizer, unsigned index) {

    if(index + 1 < tokenizer->amount) {
        token_t next = tokenizer->tokens[index + 1];

                /*5(2 + x)*/
        return  next.type == TOK_OPEN_PAR
                /*5x or x5. We can infer two number tokens will never follow each other.*/
            ||  (next.type == TOK_NUMBER || next.type == TOK_SYMBOL
                /*Crazy, but technically valid TI syntax*/
            ||  next.type == TOK_NEGATE
                /*5int(2.5)*/
            ||  is_tok_function(next.type));
    }

    return false;
}

/*How many parameters for the function or operator*/
uint8_t operand_count(TokenType type) {
    if(is_tok_unary_operator(type) || is_tok_unary_function(type))
        return 1;
    if(is_tok_binary_operator(type))
        return 2;

    if(type == TOK_LOG_BASE)
        return 2;
    
    if(type == TOK_DERIV)
        return 3;

    return 0;
}

/*Changes the TokenType into the OperatorType and fixes operands*/
void translate(ast_t *e) {

    switch((int)optype(e)) {
    case TOK_PLUS:       optype(e) = OP_ADD; break;
    case TOK_MINUS: {
        optype(e) = OP_ADD;
        ast_ChildInsert(e, ast_MakeBinary(OP_MULT, ast_MakeNumber(num_FromInt(-1)), ast_ChildRemoveIndex(e, 1)), 0);
        break;
    }
    case TOK_MULTIPLY:   optype(e) = OP_MULT; break;
    case TOK_DIVIDE:     optype(e) = OP_DIV; break;
    case TOK_FRACTION:   optype(e) = OP_DIV; break;
    case TOK_PROPER:     optype(e) = OP_ADD; break;
    case TOK_POWER:      optype(e) = OP_POW; break;
    case TOK_SCIENTIFIC: {
        ast_t *op2;
        
        optype(e) = OP_MULT;

        op2 = ast_MakeBinary(OP_POW, ast_MakeNumber(num_FromInt(10)), ast_ChildGet(e, 1));
        ast_ChildRemoveIndex(e, 1);
        ast_ChildAppend(e, op2);

        break;   
    }
    case TOK_ROOT:       optype(e) = OP_ROOT; break;
    case TOK_NEGATE:
        optype(e) = OP_MULT;
        ast_ChildInsert(e, ast_MakeNumber(num_FromInt(-1)), 0);
        break;
    case TOK_RECIPROCAL:
        optype(e) = OP_POW;
        ast_ChildAppend(e, ast_MakeNumber(num_FromInt(-1)));
        break;
    case TOK_SQUARE:
        optype(e) = OP_POW;
        ast_ChildAppend(e, ast_MakeNumber(num_FromInt(2)));
        break;
    case TOK_CUBE:
        optype(e) = OP_POW;
        ast_ChildAppend(e, ast_MakeNumber(num_FromInt(3)));
        break;
    case TOK_FACTORIAL:  optype(e) = OP_FACTORIAL; break;
    case TOK_LOG_BASE:
        optype(e) = OP_LOG;
        /*Swap the operands*/
        ast_ChildAppend(e, ast_ChildRemoveIndex(e, 0));
        break;
    case TOK_DERIV:      optype(e) = OP_DERIV; break;
    case TOK_INT:        optype(e) = OP_INT; break;
    case TOK_ABS:        optype(e) = OP_ABS; break;
    case TOK_SQRT:
        optype(e) = OP_ROOT;
        ast_ChildInsert(e, ast_MakeNumber(num_FromInt(2)), 0);
        break;
    case TOK_CUBED_ROOT:
        optype(e) = OP_ROOT;
        ast_ChildInsert(e, ast_MakeNumber(num_FromInt(3)), 0);
        break;
    case TOK_LN:
        optype(e) = OP_LOG;
        ast_ChildInsert(e, ast_MakeSymbol(SYM_EULER), 0);
        break;
    case TOK_E_TO_POWER:
        optype(e) = OP_POW;
        ast_ChildInsert(e, ast_MakeSymbol(SYM_EULER), 0);
        break;
    case TOK_LOG:
        optype(e) = OP_LOG;
        ast_ChildInsert(e, ast_MakeNumber(num_FromInt(10)), 0);
        break;
    case TOK_10_TO_POWER:
        optype(e) = OP_POW;
        ast_ChildInsert(e, ast_MakeNumber(num_FromInt(10)), 0);
        break;
    case TOK_SIN:        optype(e) = OP_SIN; break;
    case TOK_SIN_INV:    optype(e) = OP_SIN_INV; break;
    case TOK_COS:        optype(e) = OP_COS; break;
    case TOK_COS_INV:    optype(e) = OP_COS_INV; break;
    case TOK_TAN:        optype(e) = OP_TAN; break;
    case TOK_TAN_INV:    optype(e) = OP_TAN_INV; break;
    case TOK_SINH:       optype(e) = OP_SINH; break;
    case TOK_SINH_INV:   optype(e) = OP_SINH_INV; break;
    case TOK_COSH:       optype(e) = OP_COSH; break;
    case TOK_COSH_INV:   optype(e) = OP_COSH_INV; break;
    case TOK_TANH:       optype(e) = OP_TANH; break;
    case TOK_TANH_INV:   optype(e) = OP_TANH_INV; break;
    default:
        break;
    }
}

bool collapse_precedence(stack_t *operators, stack_t *expressions, TokenType type) {

    while(operators->top > 0) {
        ast_t *collapsed;
        token_t *op;
        unsigned i;

        /*Break when we meet the corresponding (*/
        if(type == TOK_CLOSE_PAR && ((token_t*)stack_Peek(operators))->type == TOK_OPEN_PAR)
            break;
        /*Break when we meet the function that the , belongs to*/
        else if(type == TOK_COMMA && is_tok_nary_function(((token_t*)stack_Peek(operators))->type))
            break;
        /*Break if the precedence is lower than the type*/
        else if(type != TOK_CLOSE_PAR && type != TOK_COMMA && precedence(((token_t*)stack_Peek(operators))->type) < precedence(type))
            break;

        op = stack_Pop(operators);

        /*Occurs when we collapse_all() at the end with not enough closing
        parentheses. This is an acceptable TI format, so we accept it too.*/
        if(op->type == TOK_OPEN_PAR)
            continue;

        /*Store the token type into the operand type. This will
        Be fixed in make_operator*/
        collapsed = ast_MakeOperator(op->type);
        for(i = 0; i < operand_count(op->type); i++) {
            ast_t *operand = stack_Pop(expressions);

            if(operand == NULL) {
                ast_Cleanup(collapsed);
                return false;
            }

            ast_ChildInsert(collapsed, operand, 0);
        }

        translate(collapsed);

        stack_Push(expressions, collapsed);
    }

    return true;
}

bool collapse_all(stack_t *operators, stack_t *expressions) {
    return collapse_precedence(operators, expressions, 0);
}

#define parse_assert(expression, err) if(!(expression)) {   \
    ast_t *current;                                         \
                                                            \
    while((current = stack_Pop(&expressions)) != NULL)      \
        ast_Cleanup(current);                               \
                                                            \
    stack_Cleanup(&operators);                              \
    stack_Cleanup(&expressions);                            \
                                                            \
    free(tokenizer.tokens);                                 \
                                                            \
    *e = err;                                               \
    return NULL;                                            \
}

#include "dbg.h"

ast_t *parse(const uint8_t *equation, unsigned length, struct Identifier *lookup, error_t *e) {
    tokenizer_t tokenizer = {0};
    stack_t operators, expressions;
    ast_t *root;

    /*Create instances to push on the stacks as pointers*/
    token_t mult =      {TOK_MULTIPLY};
    token_t open_par =  {TOK_OPEN_PAR};

    unsigned i;

    *e = tokenize(&tokenizer, equation, length, lookup);

    if(*e != E_SUCCESS)
        return NULL;

    stack_Create(&operators);
    stack_Create(&expressions);

    for(i = 0; i < tokenizer.amount; i++) {
        token_t *tok = &tokenizer.tokens[i];

        if(tok->type == TOK_OPEN_PAR) {
            stack_Push(&operators, tok);
        } else if(tok->type == TOK_NUMBER || tok->type == TOK_SYMBOL) {

            if(tok->type == TOK_NUMBER) {
                stack_Push(&expressions, ast_MakeNumber(tok->op.num));
            } else {
                stack_Push(&expressions, ast_MakeSymbol(tok->op.symbol));
            }

            if(should_multiply_by_next_token(&tokenizer, i)) {
                parse_assert(collapse_precedence(&operators, &expressions, TOK_MULTIPLY), E_PARSE_BAD_OPERATOR);
                stack_Push(&operators, &mult);
            }

        } else if(is_tok_unary_operator(tok->type)) {

            /*Or other left unary operators*/
            if(tok->type != TOK_NEGATE) {
                parse_assert(collapse_precedence(&operators, &expressions, tok->type), E_PARSE_BAD_OPERATOR);
                stack_Push(&operators, tok);

                if(should_multiply_by_next_token(&tokenizer, i)) {
                    parse_assert(collapse_precedence(&operators, &expressions, TOK_MULTIPLY), E_PARSE_BAD_OPERATOR);
                    stack_Push(&operators, &mult);
                }
            } else {
                stack_Push(&operators, tok);
            }

        } else if(is_tok_binary_operator(tok->type)) {
            parse_assert(collapse_precedence(&operators, &expressions, tok->type), E_PARSE_BAD_OPERATOR);
            stack_Push(&operators, tok);
        } else if(is_tok_function(tok->type)) {
            /*Insert a ( to correspond with the other closing ) following the parameters*/
            stack_Push(&operators, &open_par);
            stack_Push(&operators, tok);
        } else if(tok->type == TOK_CLOSE_PAR) {
            parse_assert(collapse_precedence(&operators, &expressions, TOK_CLOSE_PAR), E_PARSE_BAD_OPERATOR);
            parse_assert(operators.top > 0 && ((token_t*)stack_Peek(&operators))->type == TOK_OPEN_PAR, E_PARSE_UNMATCHED_CLOSE_PAR);

            stack_Pop(&operators);

            if(should_multiply_by_next_token(&tokenizer, i)) {
                parse_assert(collapse_precedence(&operators, &expressions, TOK_MULTIPLY), E_PARSE_BAD_OPERATOR);
                stack_Push(&operators, &mult);
            }
        } else if(tok->type == TOK_COMMA) {
            parse_assert(collapse_precedence(&operators, &expressions, TOK_COMMA), E_PARSE_BAD_OPERATOR);
            parse_assert(operators.top > 0 && is_tok_function(((token_t*)stack_Peek(&operators))->type), E_PARSE_BAD_COMMA);
        }
    }

    parse_assert(collapse_all(&operators, &expressions), E_PARSE_BAD_OPERATOR);

    root = stack_Pop(&expressions);

    parse_assert(operators.top == 0, E_GENERIC);
    parse_assert(expressions.top == 0, E_GENERIC);

    stack_Cleanup(&operators);
    stack_Cleanup(&expressions);

    free(tokenizer.tokens);

    return root;
}