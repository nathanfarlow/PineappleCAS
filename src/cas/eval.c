#include "cas.h"

static bool eval_commutative(ast_t *e) {
    /*How many numbers were accumulated. If <= 1, nothing changed*/
    unsigned num_changed = false;
    unsigned i;
    mp_rat accumulator;

    accumulator = num_FromInt(optype(e) == OP_MULT ? 1 : 0);

    for(i = 0; i < ast_ChildLength(e); i++) {
        ast_t *child = ast_ChildGet(e, i);

        if(child->type == NODE_NUMBER) {
            if(optype(e) == OP_MULT)
                mp_rat_mul(accumulator, child->op.num, accumulator);
            else
                mp_rat_add(accumulator, child->op.num, accumulator);

            ast_Cleanup(ast_ChildRemoveIndex(e, i));
            i--;
            num_changed++;
        }
    }

    /*If all children nodes were numbers or if multiplied by -*/
    if(ast_ChildLength(e) == 0
        || (optype(e) == OP_MULT && mp_rat_compare_zero(accumulator) == 0)) {
        replace_node(e, ast_MakeNumber(accumulator));
        return true;
    }
    /*Do not append accumulator if multiplying by 1 or adding 0*/
    else if(!((optype(e) == OP_ADD && mp_rat_compare_zero(accumulator) == 0)
        || (optype(e) == OP_MULT && mp_rat_compare_value(accumulator, 1, 1) == 0))) {
        ast_ChildInsert(e, ast_MakeNumber(accumulator), 0);
    }
    else
        num_Cleanup(accumulator);

    return num_changed > 1;
}

static bool eval_div(ast_t *e);

static bool eval_div_mult(ast_t *num, ast_t *den) {

    ast_t *temp_num, *temp_den, *temp_div;

    bool changed = false;
    bool div_changed = false;

    if(isoptype(num, OP_MULT)) {
        unsigned i = 0;

        for(i = 0; i < ast_ChildLength(num); i++) {

            temp_num = ast_ChildGet(num, i);

            if(isoptype(den, OP_MULT)) {
                unsigned j;

                for(j = 0; j < ast_ChildLength(den); j++) {
                    temp_den = ast_ChildGet(den, j);

                    temp_div = ast_MakeBinary(OP_DIV, ast_Copy(temp_num), ast_Copy(temp_den));

                    div_changed = eval_div(temp_div);

                    if(div_changed) {
                        ast_Cleanup(ast_ChildRemoveIndex(num, i));
                        ast_Cleanup(ast_ChildRemoveIndex(den, j));

                        ast_ChildAppend(num, temp_div);

                        changed = true;

                        j--;
                        break;
                    } else {
                        ast_Cleanup(temp_div);
                    }

                }
            } else {
                temp_den = den;

                temp_div = ast_MakeBinary(OP_DIV, ast_Copy(temp_num), ast_Copy(temp_den));

                div_changed = eval_div(temp_div);

                if(div_changed) {
                    ast_Cleanup(ast_ChildRemoveIndex(num, i));
                    replace_node(den, ast_MakeNumber(num_FromInt(1)));

                    ast_ChildAppend(num, temp_div);

                    changed = true;
                } else {
                    ast_Cleanup(temp_div);
                }

            }
        }

    } else if(isoptype(den, OP_MULT)) {
        unsigned j;

        temp_num = num;

        for(j = 0; j < ast_ChildLength(den); j++) {
            temp_den = ast_ChildGet(den, j);

            temp_div = ast_MakeBinary(OP_DIV, ast_Copy(temp_num), ast_Copy(temp_den));

            div_changed = eval_div(temp_div);

            if(div_changed) {

                replace_node(num, temp_div);
                ast_Cleanup(ast_ChildRemoveIndex(den, j));

                changed = true;

                j--;
                break;
            } else {
                ast_Cleanup(temp_div);
            }
        }

    }

    return changed;
}

static bool eval_div(ast_t *e) {
    ast_t *num, *den;

    bool changed = false;

    num = ast_ChildGet(e, 0);
    den = ast_ChildGet(e, 1);

    /*If numerator and denominator are equal, replace node with "1"*/
    if(ast_Compare(num, den)) {
        replace_node(e, ast_MakeNumber(num_FromInt(1)));
        return true;
    }

    if(num->type == NODE_NUMBER && mp_rat_compare_zero(num->op.num) == 0) {
        replace_node(e, ast_MakeNumber(num_FromInt(0)));
        return true;
    }

    if(den->type == NODE_NUMBER && mp_rat_compare_value(den->op.num, 1, 1) == 0) {
        replace_node(e, num);
        return true;
    }

    if(isoptype(num, OP_POW)) {
        ast_t *base1, *power1;

        base1 = ast_ChildGet(num, 0);
        power1 = ast_ChildGet(num, 1);

        if(isoptype(den, OP_POW)) {
            ast_t *base2, *power2;

            base2 = ast_ChildGet(den, 0);
            power2 = ast_ChildGet(den, 1);

            if(ast_Compare(base1, base2)) {
                /*Subtract powers*/
                replace_node(power1, ast_MakeBinary(OP_ADD,
                                                    ast_Copy(power1),
                                                    ast_MakeBinary(OP_MULT,
                                                                    ast_MakeNumber(num_FromInt(-1)),
                                                                    ast_Copy(power2))));
                replace_node(den, ast_MakeNumber(num_FromInt(1)));
                return true;
            }

        } else if(ast_Compare(base1, den)) {
            /*Subtract powers*/
            replace_node(power1, ast_MakeBinary(OP_ADD,
                                                ast_MakeNumber(num_FromInt(-1)),
                                                ast_Copy(power1)));
            replace_node(den, ast_MakeNumber(num_FromInt(1)));

            return true;
        }
    } else if(isoptype(den, OP_POW) && ast_Compare(num, ast_ChildGet(den, 0))) {
        /*Subtract powers*/
        ast_t *power2 = ast_ChildGet(den, 1);

        replace_node(power2, ast_MakeBinary(OP_ADD,
                                                ast_MakeNumber(num_FromInt(-1)),
                                                ast_Copy(power2)));
        replace_node(num, ast_MakeNumber(num_FromInt(1)));

        return true;
    }

    if(eval_div_mult(num, den))
        return true;

    if(num->type != NODE_NUMBER || den->type != NODE_NUMBER)
        return false;

    /*Simplify fractions using GCD*/
    if(mp_rat_is_integer(num->op.num) && mp_rat_is_integer(den->op.num)) {
        mpz_t gcd;
        mp_int a = &num->op.num->num, b = &den->op.num->num;

        /*TODO: check for division by zero?*/

        mp_int_init(&gcd);
        mp_int_gcd(a, b, &gcd);

        if(mp_int_compare_value(&gcd, 1) != 0) {
            mp_int_div(a, &gcd, a, NULL);
            mp_int_div(b, &gcd, b, NULL);
            changed = true;
        }

        mp_int_clear(&gcd);
    }

    return changed; 
}

static bool eval_pow(ast_t *e) {
    ast_t *a, *b;

    bool changed = false;

    /*a^b*/
    a = ast_ChildGet(e, 0);
    b = ast_ChildGet(e, 1);

    /*If a == 0*/
    if(is_ast_int(a, 0)) {
        /*If b != 0, e = 0*/
        if(!is_ast_int(b, 0)) {
            replace_node(e, ast_MakeNumber(num_FromInt(0)));
            return true;
        }
    }

    /*b == 0*/
    else if(is_ast_int(b, 0)) {
        /*If a != 0, e = 1*/
        if(!is_ast_int(a, 0)) {
            replace_node(e, ast_MakeNumber(num_FromInt(1)));
            return true;
        }
    }

    /*b == 1*/
    else if(is_ast_int(b, 1)) {
        replace_node(e, a);
        return true;
    }

    /*(A^B)^C = A^(BC)*/
    if(isoptype(a, OP_POW)) {
        replace_node(e, ast_MakeBinary(OP_POW,
                                        ast_Copy(ast_ChildGet(a, 0)),
                                        ast_MakeBinary(OP_MULT,
                                            ast_Copy(ast_ChildGet(a, 1)),
                                            ast_Copy(b)
                                            )
                                        ));
        return true;
    }

    /*Evaluate a^b if a and b are integers*/
    if(a->type == NODE_NUMBER && b->type == NODE_NUMBER) {

        /*TODO: add limits*/

        if(mp_rat_is_integer(a->op.num) && mp_rat_is_integer(b->op.num)) {
            mp_rat result;
            result = num_FromInt(1);

            mp_int_expt_full(&a->op.num->num, &b->op.num->num, &result->num);

            replace_node(e, ast_MakeNumber(result));

            return true;
        }

    }

    /*Do roots*/
    if(a->type == NODE_NUMBER && b->type == NODE_OPERATOR && optype(b) == OP_DIV && is_ast_int(ast_ChildGet(b, 0), 1)) {
        /*a root of b*/
        ast_t *temp = b;
        b = a;
        a = ast_ChildGet(temp, 1);

        if(mp_rat_is_integer(a->op.num) && mp_rat_is_integer(b->op.num)) {
            mp_int answer, check;
            mp_small small;
            /*Set answer = small root of a*/
            answer = mp_int_alloc();
            mp_int_init(answer);

            mp_int_to_int(&a->op.num->num, &small);

            mp_int_root(&b->op.num->num, small, answer);

            /*Check if answer ^ small == b*/
            check = mp_int_alloc();
            mp_int_init(check);

            mp_int_expt(answer, small, check);

            if(mp_int_compare(check, &b->op.num->num) == 0) {
                /*Then this was successful*/
                mp_rat ret = num_FromInt(1);
                mp_int_copy(answer, &ret->num);

                replace_node(e, ast_MakeNumber(ret));

                changed = true;
            }

            mp_int_clear(check);
            mp_int_clear(answer);

        }
    }

    return changed;
}

static bool eval_int(ast_t *e) {
    bool changed = false;
    mp_rat res;
    ast_t *a;
    
    a = ast_ChildGet(e, 0);

    if(a->type == NODE_NUMBER) {

        res = num_FromInt(1);

        mp_int_div(&a->op.num->num, &a->op.num->den, &res->num, NULL);

        replace_node(e, ast_MakeNumber(res));

        changed = true;
    }


    return changed;
}

static bool eval_abs(ast_t *e) {
    bool changed = false;
    ast_t *a = ast_ChildGet(e, 0);

    if(a->type == NODE_NUMBER) {
        changed = mp_rat_compare_zero(a->op.num) < 0;
        mp_rat_abs(a->op.num, a->op.num);
        replace_node(e, a);
    }

    return changed;
}

static bool eval_log(ast_t *e) {
    ast_t *a, *b;
    bool changed = false;

    /*log base a of b*/
    a = ast_ChildGet(e, 0);
    b = ast_ChildGet(e, 1);

    /*Ex. log base 5 of 5 = 1*/
    if(ast_Compare(a, b)) {
        replace_node(e, ast_MakeNumber(num_FromInt(1)));
        return true;
    }

    /*Ex. ln(e^5) = 5*/

    if(isoptype(b, OP_POW)) {
        ast_t *base = ast_ChildGet(b, 0);

        if(ast_Compare(a, base)) {
            replace_node(e, ast_Copy(ast_ChildGet(b, 1)));
            changed = true;
        }
    }

    return changed;
}

static bool eval_factorial(ast_t *e) {

    ast_t *a = ast_ChildGet(e, 0);

    if(a->type == NODE_NUMBER) {
        mp_rat accumulator;
        mp_int i;

        /*0! = 1*/
        if(mp_rat_compare_zero(a->op.num) == 0) {
            replace_node(e, ast_MakeNumber(num_FromInt(1)));
            return true;
        }

        if(mp_rat_is_integer(a->op.num) && mp_rat_compare_zero(a->op.num) > 0) {
            accumulator = num_FromInt(1);
            i = mp_int_alloc();
            mp_int_init_copy(i, &a->op.num->num);

            while(mp_int_compare_zero(i) != 0) {
                mp_rat_mul_int(accumulator, i, accumulator);
                mp_int_sub_value(i, 1, i);
            }

            replace_node(e, ast_MakeNumber(accumulator));

            return true;
        }

    }

    return false;
}

/*Simplifies expressions like 5 + 5 to 10*/
bool eval(ast_t *e) {

    bool changed = false;
    ast_t *current;

    if(e->type != NODE_OPERATOR)
        return false;

    /*Simplify children before simplifying parent*/
    for(current = e->op.operator.base; current != NULL; current = current->next) {
        changed |= eval(current);
    }

    if(is_op_commutative(optype(e))) {
        changed |= eval_commutative(e);
    } else {

        /*Simplify pow, root, log, factorial*/
        switch(optype(e)) {
            case OP_DIV: changed |= eval_div(e); break;
            case OP_POW: changed |= eval_pow(e); break;
            case OP_INT: changed |= eval_int(e); break;
            case OP_ABS: changed |= eval_abs(e); break;
            case OP_LOG: changed |= eval_log(e); break;
            case OP_FACTORIAL: changed |= eval_factorial(e); break;
            default:
                break;
        }
    }

    return changed;
}
