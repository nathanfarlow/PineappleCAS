#include "parser.h"

#include "stack.h"

#include "debug.h"

struct Identifier token_table[AMOUNT_TOKENS] = {
    {0, {0}}, {0, {0}},         /*TI_NUMBER, TI_SYMBOL*/

    {1, {0x70 }}, {1, {0x71 }}, /*TI_PLUS, TI_MINUS*/
    {1, {0x82 }}, {1, {0x83 }}, /*TI_MULTIPLY, TI_DIVIDES*/
    {2, {0xEF, 0x2E}},          /*TI_FRACTION*/
    {2, {0xEF, 0x2F}},          /*TI_PROPER*/
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

/*'0' through '9' and including '.'*/
#define is_num(byte) ((byte >= 0x30 && byte <= 0x39) || byte == token_table[TI_PERIOD].bytes[0])

num_t *read_num(const uint8_t *equation, unsigned index, unsigned length, unsigned *consumed) {
    num_t *num;

    unsigned size = 0;
    unsigned i;

    char *buffer;
    bool is_decimal = false;

    for(i = index; i < length; i++) {
        if(is_num(equation[i])) size++;
        else break;
    }

    num = malloc(sizeof(num_t));
    buffer = malloc(size + 1);

    /*Copy digits, but replace Ti's '.' with ascii '.'*/
    for (i = 0; i < size; i++) {
        char digit = equation[i + index];
        if(digit == token_table[TI_PERIOD].bytes[0]) {
            buffer[i] = '.';
            is_decimal = true;
        } else {
            buffer[i] = digit;
        }
    }

    buffer[size] = '\0';

    num = is_decimal ? num_CreateDecimal(buffer) : num_CreateInteger(buffer);

    free(buffer);

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

/*
Determine if we should insert a multiply operator for a symbol/number with the next token.
For example: 5(2 + 3) and 5x
*/
bool should_multiply_by_next_token(tokenizer_t *tokenizer, unsigned index) {

    if(index + 1 < tokenizer->amount) {
        token_t next = tokenizer->tokens[index + 1];

                /*5(2 + x)*/
        return  next.type == TI_OPEN_PAR
                /*5x or x5. We can infer two number tokens will never follow each other.*/
            ||  (next.type == TI_NUMBER || next.type == TI_SYMBOL
                /*Crazy, but technically valid TI syntax*/
            ||  next.type == TI_NEGATE
                /*5int(2.5)*/
            ||  is_type_function(next.type));
    }

    return false;
}

/*How many parameters for the function or operator*/
uint8_t operand_count(TokenType type) {
    if(    (type >= TI_NEGATE && type <= TI_CUBE)
        || (type >= TI_INT && type <= TI_TANH_INV))
        return 1;
    if((type >= TI_PLUS && type <= TI_ROOT) || type == TI_LOG_BASE)
        return 2;
    return 0;
}

/*Changes the TokenType into the OperatorType and fixes operands*/
void translate(ast_t *e) {

    switch((int)e->op.operator.type) {
    case TI_PLUS:       e->op.operator.type = OP_ADD; break;
    case TI_MINUS: {
        e->op.operator.type = OP_ADD;
        ast_ChildInsert(e, ast_MakeBinary(OP_MULT, ast_MakeNumber(num_CreateInteger("-1")), ast_ChildRemoveIndex(e, 1)), 0);
        break;
    }
    case TI_MULTIPLY:   e->op.operator.type = OP_MULT; break;
    case TI_DIVIDE:     e->op.operator.type = OP_DIV; break;
    case TI_FRACTION:   e->op.operator.type = OP_DIV; break;
    case TI_PROPER:     e->op.operator.type = OP_ADD; break;
    case TI_POWER:      e->op.operator.type = OP_POW; break;
    case TI_SCIENTIFIC: {
        ast_t *op2;
        
        e->op.operator.type = OP_MULT;

        op2 = ast_MakeBinary(OP_POW, ast_MakeNumber(num_CreateInteger("10")), ast_ChildGet(e, 1));
        ast_ChildRemoveIndex(e, 1);
        ast_ChildAppend(e, op2);

        break;   
    }
    case TI_ROOT:       e->op.operator.type = OP_ROOT; break;
    case TI_NEGATE:
        e->op.operator.type = OP_MULT;
        ast_ChildInsert(e, ast_MakeNumber(num_CreateInteger("-1")), 0);
        break;
    case TI_RECIPROCAL:
        e->op.operator.type = OP_POW;
        ast_ChildAppend(e, ast_MakeNumber(num_CreateInteger("-1")));
        break;
    case TI_SQUARE:
        e->op.operator.type = OP_POW;
        ast_ChildAppend(e, ast_MakeNumber(num_CreateInteger("2")));
        break;
    case TI_CUBE:
        e->op.operator.type = OP_POW;
        ast_ChildAppend(e, ast_MakeNumber(num_CreateInteger("3")));
        break;
    case TI_LOG_BASE:
        e->op.operator.type = OP_LOG;
        /*Swap the operands*/
        ast_ChildAppend(e, ast_ChildRemoveIndex(e, 0));
        break;
    case TI_INT:        e->op.operator.type = OP_INT; break;
    case TI_ABS:        e->op.operator.type = OP_ABS; break;
    case TI_SQRT:
        e->op.operator.type = OP_ROOT;
        ast_ChildInsert(e, ast_MakeNumber(num_CreateInteger("2")), 0);
        break;
    case TI_CUBED_ROOT:
        e->op.operator.type = OP_ROOT;
        ast_ChildInsert(e, ast_MakeNumber(num_CreateInteger("3")), 0);
        break;
    case TI_LN:
        e->op.operator.type = OP_LOG;
        ast_ChildInsert(e, ast_MakeSymbol(SYM_EULER), 0);
        break;
    case TI_E_TO_POWER:
        e->op.operator.type = OP_POW;
        ast_ChildInsert(e, ast_MakeSymbol(SYM_EULER), 0);
        break;
    case TI_LOG:
        e->op.operator.type = OP_LOG;
        ast_ChildInsert(e, ast_MakeNumber(num_CreateInteger("10")), 0);
        break;
    case TI_10_TO_POWER:
        e->op.operator.type = OP_POW;
        ast_ChildInsert(e, ast_MakeNumber(num_CreateInteger("10")), 0);
        break;
    case TI_SIN:        e->op.operator.type = OP_SIN; break;
    case TI_SIN_INV:    e->op.operator.type = OP_SIN_INV; break;
    case TI_COS:        e->op.operator.type = OP_COS; break;
    case TI_COS_INV:    e->op.operator.type = OP_COS_INV; break;
    case TI_TAN:        e->op.operator.type = OP_TAN; break;
    case TI_TAN_INV:    e->op.operator.type = OP_TAN_INV; break;
    case TI_SINH:       e->op.operator.type = OP_SINH; break;
    case TI_SINH_INV:   e->op.operator.type = OP_SINH_INV; break;
    case TI_COSH:       e->op.operator.type = OP_COSH; break;
    case TI_COSH_INV:   e->op.operator.type = OP_COSH_INV; break;
    case TI_TANH:       e->op.operator.type = OP_TANH; break;
    case TI_TANH_INV:   e->op.operator.type = OP_TANH_INV; break;

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
        if(type == TI_CLOSE_PAR && ((token_t*)stack_Peek(operators))->type == TI_OPEN_PAR)
            break;
        /*Break when we meet the function that the , belongs to*/
        else if(type == TI_COMMA && is_type_nnary_function(((token_t*)stack_Peek(operators))->type))
            break;
        /*Break if the precedence is lower than the type*/
        else if(type != TI_CLOSE_PAR && type != TI_COMMA && precedence(((token_t*)stack_Peek(operators))->type) < precedence(type))
            break;

        op = stack_Pop(operators);

        /*Occurs when we collapse_all() at the end with not enough closing
        parentheses. This is an acceptable TI format, so we accept it too.*/
        if(op->type == TI_OPEN_PAR)
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

            if(tok->type != TI_NEGATE && should_multiply_by_next_token(&tokenizer, i)) {
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

    parse_assert(operators.top == 0, E_GENERIC);
    parse_assert(expressions.top == 0, E_GENERIC);

    stack_Cleanup(&operators);
    stack_Cleanup(&expressions);

    free(tokenizer.tokens);

    return root;
}