#include "ast.h"

ast_t *ast_MakeNumber(num_t *num) {
    ast_t *ret = malloc(sizeof(ast_t));
    ret->type = NODE_NUMBER;
    ret->next = NULL;

    ret->op.number = num;

    return ret;
}

ast_t *ast_MakeSymbol(char symbol) {
    ast_t *ret = malloc(sizeof(ast_t));
    ret->type = NODE_SYMBOL;
    ret->next = NULL;

    ret->op.symbol = symbol;

    return ret;
}

ast_t *ast_MakeOperator(OperatorType type) {
    ast_t *ret = malloc(sizeof(ast_t));
    ret->type = NODE_OPERATOR;
    ret->next = NULL;

    ret->op.operator.type = type;
    ret->op.operator.base = NULL;

    return ret;
}

ast_t *ast_MakeUnary(OperatorType type, ast_t *operand) {
    ast_t *ret = ast_MakeOperator(type);
    ast_ChildAppend(ret, operand);
    return ret;
}

ast_t *ast_MakeBinary(OperatorType type, ast_t *left, ast_t *right) {
    ast_t *ret = ast_MakeOperator(type);
    ast_ChildAppend(ret, left);
    ast_ChildAppend(ret, right);
    return ret;
}

ast_t *ast_Copy(ast_t *e) {
    if(e == NULL)
        return NULL;

    switch(e->type) {
    case NODE_NUMBER:
        return ast_MakeNumber(num_Copy(e->op.number));
    case NODE_SYMBOL:
        return ast_MakeSymbol(e->op.symbol);
    case NODE_OPERATOR: {
        ast_t *copy, *child;

        copy = ast_MakeOperator(e->op.operator.type);
        child = e->op.operator.base;

        for(child = e->op.operator.base; child != NULL; child = child->next)
            ast_ChildAppend(copy, ast_Copy(child));

        return copy;
    }
    }

    return NULL;
}

static bool has_used(unsigned *buffer, unsigned top, unsigned index) {
    unsigned i;

    for(i = 0; i < top; i++) {
        if(buffer[i] == index)
            return true;
    }

    return false;
}

bool ast_Compare(ast_t *a, ast_t *b) {
    if(a == b)
        return true;

    if(a->type != b->type)
        return false;

    switch(a->type) {
    case NODE_NUMBER:
        if(a->op.number->is_decimal != b->op.number->is_decimal)
            return false;

        if(a->op.number->is_decimal)
            return mp_rat_compare(&a->op.number->num.rational, &b->op.number->num.rational) == 0;
        else
            return mp_int_compare(&a->op.number->num.integer, &b->op.number->num.integer) == 0;
    case NODE_SYMBOL:
        return a->op.symbol == b->op.symbol;
    case NODE_OPERATOR: {
        unsigned length;

        if(a->op.operator.type != b->op.operator.type)
            return false;

        if((length = ast_ChildLength(a)) != ast_ChildLength(b))
            return false;
        
        if(a->op.operator.type == OP_MULT || a->op.operator.type == OP_ADD) {
            unsigned a_index, b_index;
            unsigned top = 0;

            bool had_match = true;


            unsigned *buffer = malloc(sizeof(unsigned) * length);

            for(a_index = 0; a_index < length && had_match; a_index++) {

                ast_t *a_child = ast_ChildGet(a, a_index);
                had_match = false;

                for(b_index = 0; b_index < length; b_index++) {
                    ast_t *b_child;

                    if(!has_used(buffer, top, b_index)) {
                        b_child = ast_ChildGet(b, b_index);

                        if(ast_Compare(a_child, b_child)) {
                            had_match = true;
                            buffer[top++] = b_index;
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

void ast_Cleanup(ast_t *e) {
    if(e == NULL)
        return;

    switch(e->type) {
    case NODE_NUMBER:
        num_Cleanup(e->op.number);
        break;
    case NODE_SYMBOL:
        break;
    case NODE_OPERATOR: {
        /*Free each node in the list*/
        ast_t *current = e->op.operator.base;
        while(current != NULL) {
            ast_t *next = current->next;
            ast_Cleanup(current);
            current = next;
        }
        break;
    }
    }

    free(e);
}

error ast_ChildAppend(ast_t *parent, ast_t *child) {
    ast_t *last;

    if(parent->type != NODE_OPERATOR)
        return E_AST_NOT_ALLOWED;

    last = ast_ChildGetLast(parent);

    if(last == NULL)
        parent->op.operator.base = child;
    else
        last->next = child;

    child->next = NULL;
    
    return E_SUCCESS;
}

ast_t *ast_ChildGet(ast_t *parent, LSIZE index) {
    LSIZE i;
    ast_t *current;

    if(parent->type != NODE_OPERATOR)
        return NULL;

    current = parent->op.operator.base;

    for(i = 0; i <= index && current != NULL; i++) {
        if(i == index)
            return current;
        current = current->next;
    }

    return 0;
}

ast_t *ast_ChildGetLast(ast_t *parent) {
    if(parent->type != NODE_OPERATOR)
        return NULL;

    if(parent->op.operator.base == NULL) {
        return NULL;
    } else {
        ast_t *current;
        for(current = parent->op.operator.base; current->next != NULL; current = current->next);
        return current;
    }

    return NULL;
}

error ast_ChildInsert(ast_t *parent, ast_t *child, LSIZE index) {
    LSIZE i;
    ast_t *current;

    if(parent->type != NODE_OPERATOR)
        return E_AST_NOT_ALLOWED;

    if(index == 0) {
        if(parent->op.operator.base == NULL)
            parent->op.operator.base = child;
        else {
            child->next = parent->op.operator.base;
            parent->op.operator.base = child;
        }

        return E_SUCCESS;
    }

    i = 1;
    current = parent->op.operator.base;

    while(current != NULL) {

        if(i == index) {
            ast_t *temp = current->next;
            current->next = child;
            child->next = temp;

            return E_SUCCESS;
        }

        current = current->next;
        i++;
    }

    return E_AST_OUT_OF_BOUNDS;
}

ast_t *ast_ChildRemove(ast_t *parent, ast_t *child) {
    if(parent->type != NODE_OPERATOR)
        return NULL;
    return ast_ChildRemoveIndex(parent, ast_ChildIndexOf(parent, child));
}

LSIZE ast_ChildIndexOf(ast_t *parent, ast_t *child) {
    LSIZE i;
    ast_t *current;

    if(parent->type != NODE_OPERATOR)
        return -1;

    i = 0;

    for(current = parent->op.operator.base; current != NULL; current = current->next) {
        if(current == child)
            return i;
        i++;
    }

    return -1;
}

ast_t *ast_ChildRemoveIndex(ast_t *parent, LSIZE index) {
    LSIZE i;
    ast_t *current;

    if(parent->type != NODE_OPERATOR)
        return NULL;

    if(index == 0) {
        ast_t *temp;
        
        if(parent->op.operator.base == NULL)
            return NULL;

        temp = parent->op.operator.base;
        parent->op.operator.base = parent->op.operator.base->next;
        temp->next = NULL;
        return temp;
    }

    i = 1;
    current = parent->op.operator.base;

    while(current != NULL) {

        if(i == index) {
            ast_t *temp = current->next;
            current->next = temp == NULL ? NULL : temp->next;
            temp->next = NULL;
            return temp;
        }

        current = current->next;
        i++;
    }

    return NULL;
}

LSIZE ast_ChildLength(ast_t *parent) {
    LSIZE i;
    ast_t *current;

    if(parent->type != NODE_OPERATOR)
        return 0;

    i = 0;
    for(current = parent->op.operator.base; current != NULL; current = current->next)
        i++;

    return i;
}