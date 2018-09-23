#include "cas.h"
#include "mapping.h"

#include <math.h>

#ifndef _WIN32

double asinh(double x) {
    return log(x + sqrt(1 + pow(x, 2)));
}

double acosh(double x) {
    return 2 * log(sqrt((x + 1) / 2) + sqrt((x - 1) / 2));
}

double atanh(double x) {
    return (log(1 + x) - log(1 - x)) / 2;
}

#endif

static unsigned factorial(unsigned x) {
    unsigned total = 1;
    for (; x > 0; x--)
        total *= x;
    return total;
}

double approximate(ast_t *e, error_t *err) {
    *err = E_SUCCESS;

    switch(e->type) {
    case NODE_NUMBER:
        return num_ToDouble(e->op.number);
    case NODE_SYMBOL: {
        double result;
        ast_t *mapping = mapping_Get(e->op.symbol);

        if(mapping == NULL) {
            *err = E_EVAL_NO_MAPPING;
            return 0;
        }

        result = approximate(mapping, err);

        if(*err != E_SUCCESS)
            return 0;

        return result;

    } case NODE_OPERATOR: {
        OperatorType type = e->op.operator.type;

        if(is_op_nary(type)) {

            if(is_type_communative(type)) {
                ast_t *current;

                double total = (type == OP_ADD ? 0 : 1);

                for(current = e->op.operator.base; current != NULL; current = current->next) {
                    switch(type) {

                    case OP_ADD:  total += approximate(current, err); break;
                    case OP_MULT: total *= approximate(current, err); break;

                    default: break;
                    }

                    if(*err != E_SUCCESS)
                        return 0;
                }

                return total;

            } else {
                double left, right;

                left = approximate(e->op.operator.base, err);
                if(*err != E_SUCCESS) return 0;
                right = approximate(e->op.operator.base->next, err);
                if(*err != E_SUCCESS) return 0;

                switch(type) {

                case OP_DIV:    return left / right;
                case OP_POW:    return pow(left, right);
                case OP_ROOT:   return right < 0 ? -1 * pow(-1 * right, 1.0 / left) : pow(right, 1.0 / left);
                case OP_LOG:    return log(right) / log(left);

                default: break;
                }

            }

        } else {
            double x = approximate(e->op.operator.base, err);

            if(*err != E_SUCCESS) return 0;

            switch(type) {
            case OP_FACTORIAL:  return (double)factorial((unsigned)x);
            case OP_INT:        return (int)x;
            case OP_ABS:        return fabs(x);

            case OP_SIN:        return sin(x);
            case OP_SIN_INV:    return asin(x);
            case OP_COS:        return cos(x);
            case OP_COS_INV:    return acos(x);
            case OP_TAN:        return tan(x);
            case OP_TAN_INV:    return atan(x);
            case OP_SINH:       return sinh(x);
            case OP_SINH_INV:   return asinh(x);
            case OP_COSH:       return cosh(x);
            case OP_COSH_INV:   return acosh(x);
            case OP_TANH:       return tanh(x);
            case OP_TANH_INV:   return atanh(x);

            default: break;
            }
        }

        break;
    }
    }

    return 0;
}