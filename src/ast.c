#include "ast.h"

mp_rat num_FromString(const char *str) {
    mp_rat n = mp_rat_alloc();
    mp_rat_init(n);
    mp_rat_read_decimal(n, RADIX, str);
    return n;
}

mp_rat num_FromInt(mp_small num) {
    mp_rat n = mp_rat_alloc();
    mp_rat_init(n);
    mp_rat_set_value(n, num, 1);
    return n;
}

mp_rat num_FromFraction(mp_small num, mp_small den) {
    mp_rat n = mp_rat_alloc();
    mp_rat_init(n);
    mp_rat_set_value(n, num, den);
    return n;
}

mp_rat num_Copy(mp_rat other) {
    mp_rat n = mp_rat_alloc();
    mp_rat_init_copy(n, other);
    return n;
}

char *num_ToString(mp_rat num, mp_size precision) {
    mp_result res;

    char *str;
    int len;

    if(mp_rat_is_integer(num)) {
        mp_rat_reduce(num);
        len = mp_int_string_len(&num->num, RADIX);
        str = malloc(len * sizeof(char));
        if((res = mp_int_to_string(&num->num, RADIX, str, len)) != MP_OK) {
            free(str);
            return NULL;
        }
    } else {
        len = mp_rat_decimal_len(num, RADIX, precision);
        str = malloc(len * sizeof(char));
        if((res = mp_rat_to_decimal(num, RADIX, precision, MP_ROUND_HALF_UP, str, len)) != MP_OK) {
            free(str);
            return NULL;
        }
    }

    return str;
}

void num_Cleanup(mp_rat num) {
    if(num != NULL)
        mp_rat_free(num);
}

pcas_ast_t *ast_MakeNumber(mp_rat num) {
    pcas_ast_t *ret = malloc(sizeof(pcas_ast_t));
    ret->type = NODE_NUMBER;
    ret->next = NULL;

    ret->op.num = num;

    return ret;
}

pcas_ast_t *ast_MakeSymbol(char symbol) {
    pcas_ast_t *ret = malloc(sizeof(pcas_ast_t));
    ret->type = NODE_SYMBOL;
    ret->next = NULL;

    ret->op.symbol = symbol;

    return ret;
}

pcas_ast_t *ast_MakeOperator(OperatorType type) {
    pcas_ast_t *ret = malloc(sizeof(pcas_ast_t));
    ret->type = NODE_OPERATOR;
    ret->next = NULL;

    optype(ret) = type;
    opbase(ret) = NULL;

    return ret;
}

pcas_ast_t *ast_MakeUnary(OperatorType type, pcas_ast_t *operand) {
    pcas_ast_t *ret = ast_MakeOperator(type);
    ast_ChildAppend(ret, operand);
    return ret;
}

pcas_ast_t *ast_MakeBinary(OperatorType type, pcas_ast_t *left, pcas_ast_t *right) {
    pcas_ast_t *ret = ast_MakeOperator(type);
    ast_ChildAppend(ret, left);
    ast_ChildAppend(ret, right);
    return ret;
}

pcas_ast_t *ast_Copy(pcas_ast_t *e) {
    if(e == NULL)
        return NULL;

    switch(e->type) {
    case NODE_NUMBER:
        return ast_MakeNumber(num_Copy(e->op.num));
    case NODE_SYMBOL:
        return ast_MakeSymbol(e->op.symbol);
    case NODE_OPERATOR: {
        pcas_ast_t *copy, *child;

        copy = ast_MakeOperator(optype(e));
        child = opbase(e);

        for(child = opbase(e); child != NULL; child = child->next)
            ast_ChildAppend(copy, ast_Copy(child));

        return copy;
    }
    }

    return NULL;
}

static bool has_used(const unsigned *buffer, unsigned top, unsigned index) {
    unsigned i;

    for(i = 0; i < top; i++) {
        if(buffer[i] == index)
            return true;
    }

    return false;
}

bool ast_Compare(pcas_ast_t *a, pcas_ast_t *b) {
    if(a == b)
        return true;

    if(a->type != b->type)
        return false;

    switch(a->type) {
    case NODE_NUMBER:
        return mp_rat_compare(a->op.num, b->op.num) == 0;
    case NODE_SYMBOL:
        return a->op.symbol == b->op.symbol;
    case NODE_OPERATOR: {
        unsigned length;

        if(optype(a) != optype(b))
            return false;

        if((length = ast_ChildLength(a)) != ast_ChildLength(b))
            return false;
        
        /*Compare children that are not necessarily in order. O(n^2)*/
        if(optype(a) == OP_MULT || optype(a) == OP_ADD) {
            unsigned a_index, b_index;
            unsigned top = 0;

            bool had_match = true;

            /*Keep track if we have already matched a node to another node in the past*/
            unsigned *buffer = calloc(length, sizeof(unsigned));

            for(a_index = 0; a_index < length && had_match; a_index++) {

                pcas_ast_t *a_child = ast_ChildGet(a, a_index);
                had_match = false;

                for(b_index = 0; b_index < length; b_index++) {
                    pcas_ast_t *b_child;

                    if(!has_used(buffer, top, b_index)) {
                        b_child = ast_ChildGet(b, b_index);

                        if(ast_Compare(a_child, b_child)) {
                            had_match = true;
                            buffer[top++] = b_index;
                            break;
                        }
                    }
                }
            }

            free(buffer);

            return had_match;
        } else {
            unsigned i;

            for(i = 0; i < length; i++) {
                if(!ast_Compare(ast_ChildGet(a, i), ast_ChildGet(b, i)))
                    return false;
            }

            return true;
        }

        
    }
    }

    return false;
}

void ast_Cleanup(pcas_ast_t *e) {
    if(e == NULL)
        return;

    switch(e->type) {
    case NODE_NUMBER:
        num_Cleanup(e->op.num);
        break;
    case NODE_SYMBOL:
        break;
    case NODE_OPERATOR: {
        /*Free each node in the list*/
        pcas_ast_t *current = opbase(e);
        while(current != NULL) {
            pcas_ast_t *next = current->next;
            ast_Cleanup(current);
            current = next;
        }
        break;
    }
    }

    free(e);
}

pcas_error_t ast_ChildAppend(pcas_ast_t *parent, pcas_ast_t *child) {
    pcas_ast_t *last;

    if(parent->type != NODE_OPERATOR)
        return E_AST_NOT_ALLOWED;

    last = ast_ChildGetLast(parent);

    if(last == NULL)
        opbase(parent) = child;
    else
        last->next = child;

    child->next = NULL;
    
    return E_SUCCESS;
}

pcas_ast_t *ast_ChildGet(pcas_ast_t *parent, LSIZE index) {
    LSIZE i;
    pcas_ast_t *current;

    if(parent->type != NODE_OPERATOR)
        return NULL;

    current = opbase(parent);

    for(i = 0; i <= index && current != NULL; i++) {
        if(i == index)
            return current;
        current = current->next;
    }

    return 0;
}

pcas_ast_t *ast_ChildGetLast(pcas_ast_t *parent) {
    if(parent->type != NODE_OPERATOR)
        return NULL;

    if(opbase(parent) == NULL) {
        return NULL;
    } else {
        pcas_ast_t *current;
        for(current = opbase(parent); current->next != NULL; current = current->next);
        return current;
    }

    return NULL;
}

pcas_error_t ast_ChildInsert(pcas_ast_t *parent, pcas_ast_t *child, LSIZE index) {
    LSIZE i;
    pcas_ast_t *current;

    if(parent->type != NODE_OPERATOR)
        return E_AST_NOT_ALLOWED;

    if(index == 0) {
        if(opbase(parent) == NULL)
            opbase(parent) = child;
        else {
            child->next = opbase(parent);
            opbase(parent) = child;
        }

        return E_SUCCESS;
    }

    i = 1;
    current = opbase(parent);

    while(current != NULL) {

        if(i == index) {
            pcas_ast_t *temp = current->next;
            current->next = child;
            child->next = temp;

            return E_SUCCESS;
        }

        current = current->next;
        i++;
    }

    return E_AST_OUT_OF_BOUNDS;
}

pcas_ast_t *ast_ChildRemove(pcas_ast_t *parent, pcas_ast_t *child) {
    if(parent->type != NODE_OPERATOR)
        return NULL;
    return ast_ChildRemoveIndex(parent, ast_ChildIndexOf(parent, child));
}

LSIZE ast_ChildIndexOf(pcas_ast_t *parent, pcas_ast_t *child) {
    LSIZE i;
    pcas_ast_t *current;

    if(parent->type != NODE_OPERATOR)
        return -1;

    i = 0;

    for(current = opbase(parent); current != NULL; current = current->next) {
        if(current == child)
            return i;
        i++;
    }

    return -1;
}

pcas_ast_t *ast_ChildRemoveIndex(pcas_ast_t *parent, LSIZE index) {
    LSIZE i;
    pcas_ast_t *current;

    if(parent->type != NODE_OPERATOR)
        return NULL;

    if(index == 0) {
        pcas_ast_t *temp;
        
        if(opbase(parent) == NULL)
            return NULL;

        temp = opbase(parent);
        opbase(parent) = opbase(parent)->next;
        temp->next = NULL;
        return temp;
    }

    i = 1;
    current = opbase(parent);

    while(current != NULL) {

        if(i == index) {
            pcas_ast_t *temp = current->next;
            current->next = temp == NULL ? NULL : temp->next;
            if(temp != NULL)
                temp->next = NULL;
            return temp;
        }

        current = current->next;
        i++;
    }

    return NULL;
}

LSIZE ast_ChildLength(pcas_ast_t *parent) {
    LSIZE i;
    pcas_ast_t *current;

    if(parent->type != NODE_OPERATOR)
        return 0;

    i = 0;
    for(current = opbase(parent); current != NULL; current = current->next)
        i++;

    return i;
}