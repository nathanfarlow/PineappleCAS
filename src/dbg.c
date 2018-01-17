#include "dbg.h"

#ifdef DEBUG
#ifndef COMPILE_PC

void ti_debug(const char *format, ...) {
    va_list args;
    va_start(args, format);

    vsprintf(dbgout, format, args);

    va_end(args);
}

#endif
#endif

const char *operators[AMOUNT_OPS] = {
    "+", /*"-",*/ "*", "/", "^", "root", "log", "int", "abs",
    "sin", "asin", "cos", "acos", "tan", "atan",
    "sinh", "asinh", "cosh", "acosh", "tanh", "atanh"
};

const char *symbols[SYM_THETA + 1] = {
    "pi", "e", "theta"
};

void _print_tree(ast_t *e, unsigned indent, unsigned index) {
    unsigned i;
    for(i = 0; i < indent * index; i++)
        DBG((" "));

    switch(e->type) {
    case NODE_NUMBER: {
        char *num = num_ToString(e->op.number, 10);
        DBG(("NUMBER: %s\n", num));
        free(num);
        break;
    } case NODE_SYMBOL:
        if(e->op.symbol < SYM_A)
            DBG(("SYMBOL: %s\n", symbols[e->op.symbol]));
        else
            DBG(("SYMBOL: %c\n", e->op.symbol));
        break;
    case NODE_OPERATOR: {
        ast_t *current;
        DBG(("OPERATOR: %s\n", operators[e->op.operator.type]));
        for(current = e->op.operator.base; current != NULL; current = current->next) {
            _print_tree(current, indent, index + 1);
        }
        break;
    }
    }
}

void dbg_print_tree(ast_t *e, unsigned indent) {
    _print_tree(e, indent, 0);
}

unsigned dbg_count_nodes(ast_t *e) {
    unsigned amount = 1;

    if(e->type == NODE_OPERATOR) {
        ast_t *current;
        for(current = e->op.operator.base; current != NULL; current = current->next)
            amount += dbg_count_nodes(current);
    }

    return amount;
}