#include "export.h"
#include "parser.h"

#include "cas/cas.h"

#include "dbg.h"

#include <string.h>

#define RADIX 10
#define MAX_PREC 50

#define add_byte(byte) do {if(data != NULL) data[index] = (byte); index++;} while(0)
#define add_token(token) do {uint8_t i; for(i = 0; i < token_table[(token)].length; i++) add_byte(token_table[token].bytes[i]);} while(0)

static uint8_t precedence_type(OperatorType type) {
    switch(type) {
    case OP_ADD:
        return 5;
    case OP_MULT: case OP_DIV:
        return 10;
    case OP_POW: case OP_ROOT:
        return 15;
    default:
        return 0;
    }
}

static uint8_t precedence(ast_t *e) {
    if(e->type == NODE_OPERATOR)
        return precedence_type(e->op.operator.type);
    return 255;
}

#define need_paren(parent, child) ((parent->type == NODE_OPERATOR && is_type_operator(parent->op.operator.type) && precedence(child) <= precedence(parent)) || precedence(child) < precedence(parent))

/*Returns length of buffer. Writes to buffer is buffer != NULL*/
static unsigned _to_binary(ast_t *e, uint8_t *data, unsigned index, error *err) {
    
    switch(e->type) {
    case NODE_NUMBER: {
        mp_result i, len;
        char *buffer;

        if(e->op.number->is_decimal) {
            mp_result stop;

            len = mp_rat_decimal_len(&e->op.number->num.rational, RADIX, MAX_PREC);
            buffer = malloc(len);
            mp_rat_to_decimal(&e->op.number->num.rational, RADIX, MAX_PREC, MP_ROUND_HALF_UP, buffer, len);

            stop = (mp_result)strlen(buffer) - 1;
            while(stop > 0 && buffer[stop--] == '0');

            for(i = 0; i <= stop + 1; i++) {
                uint8_t c = buffer[i];
                if(c == '.')
                    c = token_table[TI_PERIOD].bytes[0];
                else if(c == '-')
                    c = token_table[TI_NEGATE].bytes[0];
                add_byte(c);
            }

        } else {
            len = mp_int_string_len(&e->op.number->num.integer, RADIX);
            buffer = malloc(len);
            mp_int_to_string(&e->op.number->num.integer, RADIX, buffer, len);

            len = (mp_result)strlen(buffer);
            
            for(i = 0; i < len; i++) {
                add_byte(buffer[i] == '-' ? token_table[TI_NEGATE].bytes[0] : buffer[i]);
            }
        }

        free(buffer);
        break;
    } case NODE_SYMBOL:

        switch(e->op.symbol) {
        case SYM_PI:    add_token(TI_PI);       break;
        case SYM_EULER: add_token(TI_EULER);    break;
        case SYM_THETA: add_token(TI_THETA);    break;
        default:        add_byte(e->op.symbol); break;
        }

        break;
    case NODE_OPERATOR: {
        unsigned i;

        switch(e->op.operator.type) {
        case OP_ADD:
        case OP_MULT: {
            ast_t *child;

            for(i = 0; i < ast_ChildLength(e) - 1; i++) {
                child = ast_ChildGet(e, i);

                if(need_paren(e, child)) add_token(TI_OPEN_PAR);
                index = _to_binary(child, data, index, err);
                if(need_paren(e, child)) add_token(TI_CLOSE_PAR);

                if(e->op.operator.type == OP_MULT) {
                    add_token(TI_MULTIPLY);
                } else {
                    add_token(TI_PLUS);
                }
                
            }

            child = ast_ChildGetLast(e);

            if(need_paren(e, child)) add_token(TI_OPEN_PAR);
            index = _to_binary(child, data, index, err);
            if(need_paren(e, child)) add_token(TI_CLOSE_PAR);

            break;
        } case OP_DIV:
          case OP_POW: {
            ast_t *a, *b;

            a = ast_ChildGet(e, 0);
            b = ast_ChildGet(e, 1);

            if(need_paren(e, a)) add_token(TI_OPEN_PAR);
            index = _to_binary(a, data, index, err);
            if(need_paren(e, a)) add_token(TI_CLOSE_PAR);

            add_token(e->op.operator.type == OP_DIV ? TI_FRACTION : TI_POWER);

            if(e->op.operator.type == OP_POW && b->type != NODE_NUMBER || need_paren(e, b)) add_token(TI_OPEN_PAR);
            index = _to_binary(b, data, index, err);
            if(e->op.operator.type == OP_POW && b->type != NODE_NUMBER || need_paren(e, b)) add_token(TI_CLOSE_PAR);

            break;
        } case OP_ROOT: {
            ast_t *a, *b;

            a = ast_ChildGet(e, 0);
            b = ast_ChildGet(e, 1);

            if(a->type == NODE_NUMBER && !a->op.number->is_decimal) {
                if(mp_int_compare_value(&a->op.number->num.integer, 2) == 0) {
                    add_token(TI_SQRT);
                    index = _to_binary(b, data, index, err);
                    add_token(TI_CLOSE_PAR);
                    break;
                } else if(mp_int_compare_value(&a->op.number->num.integer, 3) == 0) {
                    add_token(TI_CUBED_ROOT);
                    index = _to_binary(b, data, index, err);
                    add_token(TI_CLOSE_PAR);
                    break;
                }
            }

            if(need_paren(e, a)) add_token(TI_OPEN_PAR);
            index = _to_binary(a, data, index, err);
            if(need_paren(e, a)) add_token(TI_CLOSE_PAR);

            add_token(TI_ROOT);

            if(need_paren(e, b)) add_token(TI_OPEN_PAR);
            index = _to_binary(b, data, index, err);
            if(need_paren(e, b)) add_token(TI_CLOSE_PAR);

            break;
        }
        case OP_LOG: {
            ast_t *a, *b;

            a = ast_ChildGet(e, 0);
            b = ast_ChildGet(e, 1);

            if(a->type == NODE_SYMBOL && a->op.symbol == SYM_EULER) {
                add_token(TI_LN);
                index = _to_binary(b, data, index, err);
                add_token(TI_CLOSE_PAR);
                break;
            } else if(a->type == NODE_NUMBER && !a->op.number->is_decimal && mp_int_compare_value(&a->op.number->num.integer, 10) == 0) {
                add_token(TI_LOG);
                index = _to_binary(b, data, index, err);
                add_token(TI_CLOSE_PAR);
                break;
            }

            add_token(TI_LOG_BASE);
            index = _to_binary(b, data, index, err);
            add_token(TI_COMMA);
            index = _to_binary(a, data, index, err);
            add_token(TI_CLOSE_PAR);

            break;
        } default:

            switch(e->op.operator.type) {
                case OP_INT:        add_token(TI_INT);      break;
                case OP_ABS:        add_token(TI_ABS);      break;
                case OP_SIN:        add_token(TI_SIN);      break;
                case OP_SIN_INV:    add_token(TI_SIN_INV);  break;
                case OP_COS:        add_token(TI_COS);      break;
                case OP_COS_INV:    add_token(TI_COS_INV);  break;
                case OP_TAN:        add_token(TI_TAN);      break;
                case OP_TAN_INV:    add_token(TI_TAN_INV);  break;
                case OP_SINH:       add_token(TI_SINH);     break;
                case OP_SINH_INV:   add_token(TI_SINH_INV); break;
                case OP_COSH:       add_token(TI_COSH);     break;
                case OP_COSH_INV:   add_token(TI_COSH_INV); break;
                case OP_TANH:       add_token(TI_TANH);     break;
                case OP_TANH_INV:   add_token(TI_TANH_INV); break;
            }
        
            index = _to_binary(ast_ChildGet(e, 0), data, index, err);
            add_token(TI_CLOSE_PAR);
            break;
        }

        break;
    }
    }

    return index;
}

uint8_t *export_to_binary(ast_t *e, unsigned *len, error *err) {
    uint8_t *data;

    *err = E_SUCCESS;

    *len = _to_binary(e, NULL, 0, err);

    if(*err != E_SUCCESS) {
        *len = 0;
        return NULL;
    }

    data = malloc(*len);
    _to_binary(e, data, 0, err);

    return data;
}
