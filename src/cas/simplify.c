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
            
            ast_ChildGetLast(e)->next = child->op.operator.base;

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

                simplify_rational(mult);
                simplify_rational(ast_ChildGetLast(e));

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
    unsigned i, numbers = 0;
    bool has_rat = false;
    bool changed = false;
    mpz_t int_accumulator;
    mpq_t rat_accumulator;
    num_t *ret;

    if(ast_ChildLength(e) == 1) {
        ast_t *temp = ast_ChildGet(e, 0);

        e->type = temp->type;
        e->op = temp->op;

        free(temp);
        return true;
    }

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
                numbers++;
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
                numbers++;
            }
        }

        if(has_rat)
            mp_rat_add_int(&rat_accumulator, &int_accumulator, &rat_accumulator);
    }

    if(numbers > 1)
        changed = true;

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
        e->op.operator.base = NULL;
        e->type = NODE_NUMBER;
        e->op.number = ret;
        changed = true;
    } else {
        /*Don't append if we are multiplying by 1 or adding 0*/
        bool should_append = false;

        if(e->op.operator.type == OP_MULT) {
            if(ret->is_decimal) {
                mp_rat_reduce(&ret->num.rational);
                should_append = mp_rat_compare_value(&ret->num.rational, 1, 1) != 0;
            } else {
                should_append = mp_int_compare_value(&ret->num.integer, 1) != 0;
            }
        } else /*OP_MULT*/ {
            if(ret->is_decimal) {
                mp_rat_reduce(&ret->num.rational);
                should_append = mp_rat_compare_zero(&ret->num.rational) != 0;
            } else {
                should_append = mp_int_compare_zero(&ret->num.integer) != 0;
            }
        }

        if(should_append) {
            ast_ChildAppend(e, ast_MakeNumber(ret));
        } else {
            changed = numbers == 1;
            num_Cleanup(ret);
        }
    }

    return changed;
}

/*Simplifies expressions like 5 + 5 to 10*/
static bool simplify_constants(ast_t *e) {
    bool changed = false;
    ast_t *current;

    if(e->type != NODE_OPERATOR)
        return false;

    for(current = e->op.operator.base; current != NULL; current = current->next) {
        changed |= simplify_constants(current);
    }

    if(is_type_communative(e->op.operator.type)) {
        changed |= simplify_constants_communative(e);
    } else {
        /*Simplify divide and exponents*/
        switch(e->op.operator.type) {

        case OP_DIV: {
            ast_t *a, *b;
            a = ast_ChildGet(e, 0);
            b = ast_ChildGet(e, 1);

            if(ast_Compare(a, b)) {
                ast_Cleanup(a);
                ast_Cleanup(b);

                e->op.operator.base = NULL;
                e->type = NODE_NUMBER;
                e->op.number = num_CreateInteger("1");

                changed = true;
                break;
            }

            /*If a == 0*/
            if(a->type == NODE_NUMBER) {

                if((!a->op.number->is_decimal && mp_int_compare_zero(&a->op.number->num.integer) == 0)
                    || (a->op.number->is_decimal && mp_rat_compare_zero(&a->op.number->num.rational) == 0)) {

                    ast_Cleanup(a);
                    ast_Cleanup(b);

                    e->op.operator.base = NULL;
                    e->type = NODE_NUMBER;
                    e->op.number = num_CreateInteger("0");

                    changed = true;
                    break;
                }
                
            }

            /*If b == 1*/
            if(b->type == NODE_NUMBER) {
                bool is_one = false;

                if(b->op.number->is_decimal) {
                    is_one = mp_int_compare(mp_rat_numer_ref(&b->op.number->num.rational), mp_rat_denom_ref(&b->op.number->num.rational)) == 0;
                } else {
                    is_one = mp_int_compare_value(&b->op.number->num.integer, 1) == 0;
                }

                if(is_one) {
                    ast_Cleanup(b);

                    e->type = a->type;
                    e->op = a->op;
                    
                    free(a);

                    changed = true;
                    break;
                }

            }

            if(a->type != NODE_NUMBER || b->type != NODE_NUMBER)
                break;

            if(!a->op.number->is_decimal && !b->op.number->is_decimal) {
                mpz_t gcd;
                mp_int_init(&gcd);
                mp_int_gcd(&a->op.number->num.integer, &b->op.number->num.integer, &gcd);

                if(mp_int_compare_value(&gcd, 1) != 0) {
                    mp_int_div(&a->op.number->num.integer, &gcd, &a->op.number->num.integer, NULL);
                    mp_int_div(&b->op.number->num.integer, &gcd, &b->op.number->num.integer, NULL);
                    changed = true;
                }

                mp_int_clear(&gcd);
            }
            break;
        } case OP_POW: {
            ast_t * a, *b;
            a = ast_ChildGet(e, 0);
            b = ast_ChildGet(e, 1);

            /*If a == 0*/
            if(a->type == NODE_NUMBER) {

                if((!a->op.number->is_decimal && mp_int_compare_zero(&a->op.number->num.integer) == 0)
                    || (a->op.number->is_decimal && mp_rat_compare_zero(&a->op.number->num.rational) == 0)) {

                    ast_Cleanup(a);
                    ast_Cleanup(b);

                    e->op.operator.base = NULL;
                    e->type = NODE_NUMBER;
                    e->op.number = num_CreateInteger("0");

                    changed = true;
                    break;
                }
            }

            /*If b == 1*/
            if(b->type == NODE_NUMBER) {
                bool is_one = false;

                if(b->op.number->is_decimal) {
                    is_one = mp_int_compare(mp_rat_numer_ref(&b->op.number->num.rational), mp_rat_denom_ref(&b->op.number->num.rational)) == 0;
                } else {
                    is_one = mp_int_compare_value(&b->op.number->num.integer, 1) == 0;
                }

                if(is_one) {
                    ast_Cleanup(b);

                    e->type = a->type;
                    e->op = a->op;

                    free(a);

                    changed = true;
                    break;
                }

            }

            if(a->type != NODE_NUMBER || b->type != NODE_NUMBER)
                break;

            if(!a->op.number->is_decimal && !b->op.number->is_decimal) {
                num_t *result;

                /*Only evaluate numbers up to the 10th power.*/
                if(mp_int_compare_value(&b->op.number->num.integer, 10) > 0)
                    break;

                result = malloc(sizeof(num_t));
                result->is_decimal = false;
                mp_int_init(&result->num.integer);

                mp_int_expt_full(&a->op.number->num.integer, &b->op.number->num.integer, &result->num.integer);

                ast_Cleanup(a);
                ast_Cleanup(b);

                e->op.operator.base = NULL;
                e->type = NODE_NUMBER;
                e->op.number = result;
                
                changed = true;
            }
            break;
        } case OP_ROOT: {
            ast_t * a, *b;
            a = ast_ChildGet(e, 0);
            b = ast_ChildGet(e, 1);

            /*If b == 0*/
            if(b->type == NODE_NUMBER) {

                if((!b->op.number->is_decimal && mp_int_compare_zero(&b->op.number->num.integer) == 0)
                    || (b->op.number->is_decimal && mp_rat_compare_zero(&b->op.number->num.rational) == 0)) {

                    ast_Cleanup(a);
                    ast_Cleanup(b);

                    e->op.operator.base = NULL;
                    e->type = NODE_NUMBER;
                    e->op.number = num_CreateInteger("0");

                    changed = true;
                    break;
                }
            }

            /*If a == 1*/
            if(a->type == NODE_NUMBER) {
                bool is_one = false;

                if(a->op.number->is_decimal) {
                    is_one = mp_int_compare(mp_rat_numer_ref(&a->op.number->num.rational), mp_rat_denom_ref(&a->op.number->num.rational)) == 0;
                } else {
                    is_one = mp_int_compare_value(&a->op.number->num.integer, 1) == 0;
                }

                if(is_one) {
                    ast_Cleanup(a);

                    e->type = b->type;
                    e->op = b->op;

                    free(b);

                    changed = true;
                    break;
                }

            }

            /*If b == 1*/
            if(b->type == NODE_NUMBER) {
                bool is_one = false;

                if(b->op.number->is_decimal) {
                    is_one = mp_int_compare(mp_rat_numer_ref(&b->op.number->num.rational), mp_rat_denom_ref(&b->op.number->num.rational)) == 0;
                } else {
                    is_one = mp_int_compare_value(&b->op.number->num.integer, 1) == 0;
                }

                if(is_one) {
                    ast_Cleanup(a);

                    e->type = b->type;
                    e->op = b->op;

                    free(b);

                    changed = true;
                    break;
                }

            }

            if(a->type == NODE_NUMBER && b->type == NODE_NUMBER
                && !a->op.number->is_decimal && !b->op.number->is_decimal) {
                mp_small out;
                mpz_t check;

                num_t *ret = malloc(sizeof(num_t));
                ret->is_decimal = false;

                mp_int_to_int(&a->op.number->num.integer, &out);

                mp_int_init(&ret->num.integer);
                mp_int_root(&b->op.number->num.integer, out, &ret->num.integer);

                mp_int_init(&check);
                mp_int_expt(&ret->num.integer, out, &check);

                if(mp_int_compare(&check, &b->op.number->num.integer) == 0) {

                    e->op.operator.base = NULL;
                    e->type = NODE_NUMBER;
                    e->op.number = ret;

                    ast_Cleanup(a);
                    ast_Cleanup(b);
                }

                mp_int_clear(&check);
            }

            break;
        } case OP_INT: {
            ast_t *child = ast_ChildGet(e, 0);

            if(child->type != NODE_NUMBER)
                break;

            if(child->op.number->is_decimal) {
                num_t *result;

                result = malloc(sizeof(num_t));
                result->is_decimal = false;

                mp_int_init(&result->num.integer);

                mp_int_div(mp_rat_numer_ref(&child->op.number->num.rational), mp_rat_denom_ref(&child->op.number->num.rational), &result->num.integer, NULL);

                if(mp_rat_sign(&child->op.number->num.rational) == 1) {
                    mp_int_sub_value(&result->num.integer, 1, &result->num.integer);
                    result->num.integer.sign = 1;
                }

                ast_Cleanup(child);

                e->op.operator.base = NULL;
                e->type = NODE_NUMBER;
                e->op.number = result;
            } else {
                e->op.operator.base = NULL;
                e->type = child->type;
                e->op = child->op;

                free(child);
            }

            changed = true;

            break;
        } case OP_ABS: {
            ast_t *child = ast_ChildGet(e, 0);

            if(child->type != NODE_NUMBER)
                break;

            if(child->op.number->is_decimal) {
                mp_rat_abs(&child->op.number->num.rational, &child->op.number->num.rational);
            } else {
                mp_int_abs(&child->op.number->num.integer, &child->op.number->num.integer);
            }

            e->op.operator.base = NULL;
            e->type = child->type;
            e->op = child->op;

            free(child);

            changed = true;

            break;
        } default:
            break;
        }
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

static bool simplify_identities(ast_t *e) {
    ast_t *current;
    bool changed = false;

    if(e->type != NODE_OPERATOR)
        return false;

    for(current = e->op.operator.base; current != NULL; current = current->next) {
        changed |= simplify_identities(current);
    }

    switch(e->op.operator.type) {
    case OP_POW: {
        ast_t *a, *b;
        a = ast_ChildGet(e, 0);
        b = ast_ChildGet(e, 1);

        if(b->type == NODE_OPERATOR && b->op.operator.type == OP_LOG) {
            if(ast_Compare(a, ast_ChildGet(b, 0))) {
                ast_t *simp = ast_ChildRemoveIndex(b, 1);

                e->type = simp->type;
                e->op = simp->op;

                free(simp);

                ast_Cleanup(a);
                ast_Cleanup(b);

                changed = true;
                break;
            }
        }

        if(a->type == NODE_OPERATOR && a->op.operator.type == OP_ROOT) {
            if(ast_Compare(b, ast_ChildGet(a, 0))) {
                ast_t *simp = ast_ChildRemoveIndex(a, 1);

                e->type = simp->type;
                e->op = simp->op;

                free(simp);

                ast_Cleanup(a);
                ast_Cleanup(b);

                changed = true;
                break;
            }
        }

        break;
    } case OP_ROOT: {
        ast_t *a, *b;
        a = ast_ChildGet(e, 0);
        b = ast_ChildGet(e, 1);

        if(b->type == NODE_OPERATOR && b->op.operator.type == OP_POW) {
            if(ast_Compare(a, ast_ChildGet(b, 1))) {
                ast_t *simp = ast_ChildRemoveIndex(b, 0);

                e->type = simp->type;
                e->op = simp->op;

                free(simp);

                ast_Cleanup(a);
                ast_Cleanup(b);

                changed = true;
                break;
            }
        }

        break;
    } case OP_LOG: {
        ast_t *a, *b;
        a = ast_ChildGet(e, 0);
        b = ast_ChildGet(e, 1);

        if(ast_Compare(a, b)) {
            ast_Cleanup(a);
            ast_Cleanup(b);

            e->op.operator.base = NULL;
            e->type = NODE_NUMBER;
            e->op.number = num_CreateInteger("1");

            changed = true;
            break;
        }

        if(b->type == NODE_OPERATOR && b->op.operator.type == OP_POW) {
            if(ast_Compare(a, ast_ChildGet(b, 0))) {
                ast_t *simp = ast_ChildRemoveIndex(b, 1);

                e->type = simp->type;
                e->op = simp->op;

                free(simp);

                ast_Cleanup(a);
                ast_Cleanup(b);

                changed = true;
            }
        }

        break;
    }
    /*TODO*/
    case OP_SIN: break;
    case OP_SIN_INV: break;
    case OP_COS: break;
    case OP_COS_INV: break;
    case OP_TAN: break;
    case OP_TAN_INV: break;
    case OP_SINH: break;
    case OP_SINH_INV: break;
    case OP_COSH: break;
    case OP_COSH_INV: break;
    case OP_TANH: break;
    case OP_TANH_INV: break;
    default: break;
    }

    return changed;
}

/*Rewrite X * X^2 as X^1 * X^2*/
static void _transform_multiplecation(ast_t *e) {
    unsigned i;

    if(e->type != NODE_OPERATOR)
        return;

    if(e->op.operator.type != OP_MULT)
        return;

    for(i = 0; i < ast_ChildLength(e); i++) {
        ast_t *current = ast_ChildGet(e, i);
        if((current->type == NODE_OPERATOR && current->op.operator.type != OP_POW) || current->type == NODE_SYMBOL) {
            ast_t *temp = malloc(sizeof(ast_t));

            temp->type = current->type;
            temp->op = current->op;
            temp->next = NULL;
            
            current->type = NODE_OPERATOR;
            current->op.operator.type = OP_POW;
            current->op.operator.base = NULL;
            
            ast_ChildAppend(current, temp);
            ast_ChildAppend(current, ast_MakeNumber(num_CreateInteger("1")));

            _transform_multiplecation(temp);
        } else {
            _transform_multiplecation(current);
        }
    }
}

/*Rewrite X + 2*X as 1*X + 2*X*/
static void _transform_addition(ast_t *e) {
    ast_t *current;

    if(e->type != NODE_OPERATOR)
        return;

    if(e->op.operator.type != OP_ADD)
        return;

    for(current = e->op.operator.base; current != NULL; current = current->next) {
        if((current->type == NODE_OPERATOR && current->op.operator.type != OP_MULT) || current->type == NODE_SYMBOL) {
            ast_t *temp = malloc(sizeof(ast_t));

            temp->type = current->type;
            temp->op = current->op;
            temp->next = NULL;

            current->type = NODE_OPERATOR;
            current->op.operator.type = OP_MULT;
            current->op.operator.base = NULL;

            ast_ChildAppend(current, ast_MakeNumber(num_CreateInteger("1")));
            ast_ChildAppend(current, temp);

            _transform_addition(temp);
        } else {
            _transform_addition(current);
        }
    }
}

static bool simplify_like_terms(ast_t *e) {
    bool changed = false;
    unsigned x, y, len;

    if(e->type != NODE_OPERATOR)
        return false;

    if(e->op.operator.type == OP_MULT) {
        _transform_multiplecation(e);

        for(x = 0; x < (len = ast_ChildLength(e)); x++) {
            ast_t *a = ast_ChildGet(e, x);

            if(a->type != NODE_OPERATOR || a->op.operator.type != OP_POW)
                continue;

            for(y = 0; y < len; y++) {
                ast_t *b;
                if(x == y)
                    continue;
                b = ast_ChildGet(e, y);

                if(b->type != NODE_OPERATOR || b->op.operator.type != OP_POW)
                    continue;

                if(ast_Compare(ast_ChildGet(a, 0), ast_ChildGet(b, 0))) {
                    ast_t *add;

                    add = ast_MakeBinary(OP_ADD, ast_ChildRemoveIndex(a, 1), ast_ChildRemoveIndex(b, 1));

                    ast_ChildAppend(a, add);
                    ast_Cleanup(ast_ChildRemoveIndex(e, y));

                    changed = true;
                    break;
                }
            }
        }

        while(simplify_constants(e));
    } else if(e->op.operator.type == OP_ADD) {
        _transform_addition(e);

        for(x = 0; x < (len = ast_ChildLength(e)); x++) {
            ast_t *a = ast_ChildGet(e, x);

            if(a->type != NODE_OPERATOR || a->op.operator.type != OP_MULT)
                continue;

            for(y = 0; y < ast_ChildLength(e); y++) {
                unsigned i, j;
                ast_t *b;

                if(x == y)
                    continue;
                b = ast_ChildGet(e, y);

                if(b->type != NODE_OPERATOR || b->op.operator.type != OP_MULT)
                    continue;

                for(i = 0; i < ast_ChildLength(a); i++) {
                    ast_t *f = ast_ChildGet(a, i);
                    for(j = 0; j < ast_ChildLength(b); j++) {
                        ast_t *g = ast_ChildGet(b, j);

                        if(f->type != NODE_NUMBER && g->type != NODE_NUMBER && ast_Compare(f, g)) {
                            ast_Cleanup(ast_ChildRemove(b, g));
                            ast_ChildRemove(a, f);
                            ast_ChildRemove(e, a);

                            a->op.operator.type = OP_ADD;
                            ast_ChildGetLast(a)->next = b->op.operator.base;
                            b->op.operator.base = NULL;

                            ast_ChildInsert(e, ast_MakeBinary(OP_MULT, a, f), 0);

                            ast_Cleanup(ast_ChildRemoveIndex(e, y));

                            x--;

                            changed = true;

                            break;
                        }
                    }
                }
            }
        }

        while(simplify_constants(e));
    }

    for(x = 0; x < ast_ChildLength(e); x++)
        changed |= simplify_like_terms(ast_ChildGet(e, x));

    return changed;
}

bool simplify(ast_t *e) {
    bool changed = false;

    while(simplify_communative(e))
        changed = true;

    while(simplify_rational(e))
        changed = true;

    while(simplify_rational_num(e))
        changed = true;

    while(simplify_identities(e))
        changed = true;

    while(simplify_constants(e))
        changed = true;

    while(simplify_like_terms(e))
        changed = true;

    return changed;
}