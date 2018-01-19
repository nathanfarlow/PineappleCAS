#include "cas.h"

#include <stdbool.h>

/*
Scrambles the order of the parameters, but that's fine
because we're going to put them in canonical form anyway.

Returns true if the structure of the ast was changed
*/
static bool simplify_communative(ast_t *e) {
    unsigned i;
    bool changed = false;

    if(e->type != NODE_OPERATOR)
        return false;

    for(i = 0; i < ast_ChildLength(e); i++) {

        ast_t *child = ast_ChildGet(e, i);

        if(is_type_communative(e->op.operator.type)
            && child->type == NODE_OPERATOR 
            && child->op.operator.type == e->op.operator.type) {
            
            ast_ChildAppend(e, child->op.operator.base);

            ast_ChildRemoveIndex(e, i);
            child->op.operator.base = NULL;
            ast_Cleanup(child);

            changed = true;
            i--;
        } else {
            changed |= simplify_communative(child);
        }
    }

    return changed;
}

static bool simplify_rational(ast_t *e) {
    unsigned i;
    bool changed = false;

    if(e->type != NODE_OPERATOR)
        return false;

    for(i = 0; i < ast_ChildLength(e); i++) {
        ast_t *child = ast_ChildGet(e, i);

        if(child->type == NODE_OPERATOR && child->op.operator.type == OP_DIV) {

            if(e->op.operator.type == OP_DIV) {

                if(i == 0) {
                    /*case 1*/
                    ast_ChildRemoveIndex(e, i);

                    ast_ChildInsert(e, ast_ChildRemoveIndex(child, 0), 0);
                    ast_ChildAppend(e, ast_MakeBinary(OP_MULT,  ast_ChildRemoveIndex(child, 0), ast_ChildRemoveIndex(e, 1)));

                    ast_Cleanup(child);

                    changed = true;
                    i--;
                } else {
                    /*case 2*/
                    ast_ChildRemoveIndex(e, i);

                    ast_ChildAppend(e, ast_ChildRemoveIndex(child, 0));
                    ast_ChildInsert(e, ast_MakeBinary(OP_MULT, ast_ChildRemoveIndex(child, 0), ast_ChildRemoveIndex(e, 0)), 0);
                    ast_Cleanup(child);

                    changed = true;
                    i--;
                }

            } else if(e->op.operator.type == OP_MULT) {
                /*case 3*/
                ast_t *mult;

                mult = ast_MakeOperator(OP_MULT);
                e->op.operator.type = OP_DIV;

                ast_ChildRemoveIndex(e, i);

                mult->op.operator.base = e->op.operator.base;
                e->op.operator.base = NULL;

                ast_ChildAppend(mult, ast_ChildRemoveIndex(child, 0));
                ast_ChildAppend(e, mult);
                ast_ChildAppend(e, ast_ChildRemoveIndex(child, 0));

                ast_Cleanup(child);

                changed = true;
                i--;
            }
        } else {
            changed |= simplify_rational(child);
        }
    }

    return changed;
}

static bool simplify_constants_communative(ast_t *e) {
    unsigned i, changed = 0;
    bool has_rat = false;
    mpz_t int_accumulator;
    mpq_t rat_accumulator;
    num_t *ret;

    if(e->op.operator.type == OP_MULT) {
        mp_int_init_value(&int_accumulator, 1);
        mp_rat_init(&rat_accumulator);
        mp_rat_set_value(&rat_accumulator, 1, 1);

        for(i = 0; i < ast_ChildLength(e); i++) {
            ast_t *child = ast_ChildGet(e, i);

            if(child->type == NODE_NUMBER) {

                if(child->op.number->is_decimal) {
                    mp_rat_mul(&rat_accumulator, &child->op.number->num.rational, &rat_accumulator);
                    has_rat = true;
                } else {
                    mp_int_mul(&int_accumulator, &child->op.number->num.integer, &int_accumulator);
                }

                ast_Cleanup(ast_ChildRemoveIndex(e, i));
                i--;
                changed++;
            }
        }

        if(has_rat)
            mp_rat_mul_int(&rat_accumulator, &int_accumulator, &rat_accumulator);

    } else /*OP_ADD*/ {
        mp_int_init(&int_accumulator);
        mp_int_zero(&int_accumulator);
        mp_rat_init(&rat_accumulator);
        mp_rat_zero(&rat_accumulator);

        for(i = 0; i < ast_ChildLength(e); i++) {
            ast_t *child = ast_ChildGet(e, i);

            if(child->type == NODE_NUMBER) {

                if(child->op.number->is_decimal) {
                    mp_rat_add(&rat_accumulator, &child->op.number->num.rational, &rat_accumulator);
                    has_rat = true;
                } else {
                    mp_int_add(&int_accumulator, &child->op.number->num.integer, &int_accumulator);
                }
                
                ast_Cleanup(ast_ChildRemoveIndex(e, i));
                i--;
                changed++;
            }
        }

        if(has_rat)
            mp_rat_add_int(&rat_accumulator, &int_accumulator, &rat_accumulator);
    }

    ret = malloc(sizeof(num_t));
    ret->is_decimal = has_rat;

    if(has_rat)
        ret->num.rational = rat_accumulator;
    else {
        mp_rat_clear(&rat_accumulator);

        mp_int_init(&ret->num.integer);
        mp_int_copy(&int_accumulator, &ret->num.integer);

        mp_int_clear(&int_accumulator);
    }

    if(ast_ChildLength(e) == 0) {
        e->type = NODE_NUMBER;
        e->op.number = ret;
        e->op.operator.base = NULL;
    } else {
        ast_ChildAppend(e, ast_MakeNumber(ret));
    }

    return changed > 1;
}

/*Simplifies expressions like 5 + 5 to 10*/
static bool simplify_constants(ast_t *e) {
    bool changed = false;
    ast_t *current;

    if(e->type != NODE_OPERATOR)
        return false;

    if(is_type_communative(e->op.operator.type)) {
        changed |= simplify_constants_communative(e);
    } else {
        /*Simplify divide*/
    }

    for(current = e->op.operator.base; current != NULL; current = current->next) {
        changed |= simplify_constants(current);
    }

    return changed;
}

static bool simplify_rational_num(ast_t *e) {
    bool changed = false;
    ast_t *current;

    if(e->type == NODE_NUMBER) {

        if(e->op.number->is_decimal) {
            num_t *numer, *denom;

            numer = malloc(sizeof(num_t));
            numer->is_decimal = false;
            mp_int_init(&numer->num.integer);
            mp_rat_numer(&e->op.number->num.rational, &numer->num.integer);

            denom = malloc(sizeof(num_t));
            denom->is_decimal = false;
            mp_int_init(&denom->num.integer);
            mp_rat_denom(&e->op.number->num.rational, &denom->num.integer);

            num_Cleanup(e->op.number);

            e->type = NODE_OPERATOR;
            e->op.operator.type = OP_DIV;
            e->op.operator.base = NULL;

            ast_ChildAppend(e, ast_MakeNumber(numer));
            ast_ChildAppend(e, ast_MakeNumber(denom));

            changed = true;
        }

    } else if(e->type == NODE_OPERATOR) {
        for(current = e->op.operator.base; current != NULL; current = current->next) {
            changed |= simplify_rational_num(current);
        }
    }

    return changed;
}

static bool _simplify(ast_t *e) {
    bool changed = false;

    while(simplify_communative(e))
        changed = true;

    while(simplify_rational(e))
        changed = true;

    while(simplify_constants(e))
        changed = true;

    while(simplify_rational_num(e))
        changed = true;

    return changed;
}

bool simplify(ast_t *e) {
    bool changed = false;

    while(_simplify(e))
        changed = true;

    return changed;
}