#include "cas.h"
#include "identities.h"

/*Executes the SIMP_COMMUTATIVE flag*/
bool simplify_commutative(ast_t *e) {
    unsigned i;
    bool changed = false;

    if(e->type != NODE_OPERATOR)
        return false;

    if(ast_ChildLength(e) == 1
        && (optype(e) == OP_MULT || optype(e) == OP_ADD)) {
        replace_node(e, ast_ChildGet(e, 0));
        simplify_commutative(e);
        return true;
    }

    for(i = 0; i < ast_ChildLength(e); i++) {

        ast_t *child = ast_ChildGet(e, i);

        /*If this child is the same type as parent and
        the parent's type is commutative*/
        if(child->type == NODE_OPERATOR 
            && is_op_commutative(optype(e))
            && optype(child) == optype(e)) {
            
            /*Append children of child to end of parent*/
            ast_ChildGetLast(e)->next = child->op.operator.base;
            /*Remove child node*/
            ast_ChildRemoveIndex(e, i);
            /*Remove reference to our new children so we can dealloc it*/
            child->op.operator.base = NULL;
            ast_Cleanup(child);

            changed = true;
            i--;
        } else {
            changed |= simplify_commutative(child);
        }
    }

    return changed;
}

/*Change a to b and delete b*/
void replace_node(ast_t *a, ast_t *b) {

    if(a->type == NODE_OPERATOR) {

        while(opbase(a) != NULL) {
            ast_t *child = ast_ChildRemoveIndex(a, 0);
            if(child != b)
                ast_Cleanup(child);
        }

    } else if(a->type == NODE_NUMBER) {
        num_Cleanup(a->op.num);
    }

    a->type = b->type;
    a->op = b->op;

    free(b);
}

/*Executes the SIMP_RATIONAL flag*/
bool simplify_rational(ast_t *e) {
    unsigned i;
    bool changed = false;

    if(e->type != NODE_OPERATOR)
        return false;

    for(i = 0; i < ast_ChildLength(e); i++) {
        ast_t *child = ast_ChildGet(e, i);

        if(isoptype(child, OP_DIV)) {
            ast_t *child_num, *child_den;

            child_num = ast_ChildGet(child, 0);
            child_den = ast_ChildGet(child, 1);

            if(optype(e) == OP_DIV) {
                ast_t *e_num, *e_den;

                e_num = ast_ChildGet(e, 0);
                e_den = ast_ChildGet(e, 1);

                /*
                    If the child that is a division node is the numerator of this division node
                    (A/B)/C = A/(B*C)
                */
                if(i == 0) {

                    ast_t *new_num, *new_den;

                    new_num = ast_Copy(child_num);
                    new_den = ast_MakeBinary(OP_MULT,
                                                     ast_Copy(child_den),
                                                     ast_Copy(e_den));

                    replace_node(e, ast_MakeBinary(OP_DIV, new_num, new_den));

                    changed = true;
                    /*Decrement i so we continue to simplify this node*/
                    i--;
                }
                /*
                    If the child that is a division node is the denominator of this division node
                    A/(B/C) = (A*C)/B
                */ 
                else {

                    ast_t *new_num, *new_den;

                    new_num = ast_MakeBinary(OP_MULT,
                                                   ast_Copy(e_num),
                                                   ast_Copy(child_den));
                    new_den = ast_Copy(child_num);

                    replace_node(e, ast_MakeBinary(OP_DIV, new_num, new_den));

                    changed = true;
                    i--;
                }

            } 
            /*
                A * (B / C) = (A * B) / C
                (A / B) * C = (C * A) / B
            */
            else if(optype(e) == OP_MULT) {
                ast_t *mult, *num_copy, *den_copy;
            
                mult = ast_Copy(e);
                num_copy = ast_Copy(child_num);
                den_copy = ast_Copy(child_den);

                /*Multiply by division denominator*/
                ast_ChildAppend(mult, num_copy);

                /*Remove division node*/
                ast_Cleanup(ast_ChildRemoveIndex(mult, i));

                replace_node(e, ast_MakeBinary(OP_DIV, mult, den_copy));

                /*Continue to simplify the other multiplication nodes now that our type has changed*/
                simplify_rational(mult);
                simplify_rational(ast_ChildGetLast(e));

                changed = true;
                i--;
            } else {
                changed |= simplify_rational(child);
            }

        } else {
            changed |= simplify_rational(child);
        }
    }

    return changed;
}

/*Executes the SIMP_NORMALIZE flag*/
bool simplify_normalize(ast_t *e) {
    ast_t *child;
    bool changed = false;

    if(e->type == NODE_OPERATOR) {

        for(child = opbase(e); child != NULL; child = child->next)
            changed |= simplify_normalize(child);

        if(optype(e) == OP_ROOT) {
            /*a root of b*/
            ast_t *a, *b;

            a = ast_Copy(ast_ChildGet(e, 0));
            b = ast_Copy(ast_ChildGet(e, 1));

            replace_node(e, ast_MakeBinary(OP_POW,
                                b,
                                ast_MakeBinary(OP_DIV,
                                    ast_MakeNumber(num_FromInt(1)),
                                    a
            )));

            changed = true;
        } else if(optype(e) == OP_POW) {
            ast_t *base, *power;

            base = ast_ChildGet(e, 0);
            power = ast_ChildGet(e, 1);

            if(isoptype(base, OP_MULT)) {
                unsigned j;
                ast_t *replacement = ast_MakeOperator(OP_MULT);

                for(j = 0; j < ast_ChildLength(base); j++) {
                    ast_t *cur = ast_ChildGet(base, j);

                    ast_ChildAppend(replacement, ast_MakeBinary(OP_POW,
                                                                ast_Copy(cur),
                                                                ast_Copy(power)
                                                                ));
                }

                replace_node(e, replacement);
                changed = true;
            }
        }
    } else if(e->type == NODE_NUMBER && !mp_rat_is_integer(e->op.num)) {
        mp_rat num, den;

        num = num_FromInt(1);
        den = num_FromInt(1);

        mp_rat_reduce(e->op.num);

        mp_int_copy(&e->op.num->num, &num->num);
        mp_int_copy(&e->op.num->den, &den->num);

        replace_node(e, ast_MakeBinary(OP_DIV, ast_MakeNumber(num), ast_MakeNumber(den)));


        changed = true;
    }
    
    return changed;
}

bool has_imaginary_node(ast_t *e) {
    ast_t *child;

    if(e->type == NODE_SYMBOL && e->op.symbol == SYM_IMAG)
        return true;

    if(e->type == NODE_OPERATOR)
        for(child = ast_ChildGet(e, 0); child != NULL; child = child->next)
            if(has_imaginary_node(child))
                return true;

    return false;
}

/*Expects everything to be completely simplified*/
bool is_negative_for_sure(ast_t *a) {
    if(a->type == NODE_NUMBER && mp_rat_compare_zero(a->op.num) < 0)
        return true;

    if(isoptype(a, OP_MULT)) {
        ast_t *child;
        for(child = ast_ChildGet(a, 0); child != NULL; child = child->next)
            if(is_negative_for_sure(child))
                return true;
    } else if(isoptype(a, OP_DIV)) {
        return is_negative_for_sure(ast_ChildGet(a, 0)) || is_negative_for_sure(ast_ChildGet(a, 1));
    }

    return false;
}

/*Returns true if changed. Expects completely simplified.*/
bool absolute_val(ast_t *e) {
    if(e->type == NODE_NUMBER && mp_rat_compare_zero(e->op.num) < 0) {
        mp_rat_abs(e->op.num, e->op.num);
        return true;
    }

    if(isoptype(e, OP_MULT)) {
        ast_t *child;
        for(child = ast_ChildGet(e, 0); child != NULL; child = child->next)
            if(absolute_val(child))
                return true;
    } else if(isoptype(e, OP_DIV)) {
        return absolute_val(ast_ChildGet(e, 0)) || absolute_val(ast_ChildGet(e, 1));
    }

    return false;
}

/*returns negative if a < b, 0 if a=b, positive if a > b in terms of sorting order*/
int compare(ast_t *a, ast_t *b, bool add) {
    ast_t *temp;
    int multiplier = 1;

    if(isoptype(b, OP_MULT)) {
        temp = a;
        a = b;
        b = temp;
        multiplier *= -1;
    }

    if(isoptype(a, OP_MULT)) {
        ast_t *g = gcd(a, b);

        if(!is_ast_int(g, 1)) {
            ast_t *newa, *newb;
            int val;

            newa = ast_MakeBinary(OP_DIV, ast_Copy(a), ast_Copy(g));
            newb = ast_MakeBinary(OP_DIV, ast_Copy(b), ast_Copy(g));

            simplify(newa, SIMP_NORMALIZE | SIMP_RATIONAL | SIMP_EVAL);
            simplify(newb, SIMP_NORMALIZE | SIMP_RATIONAL | SIMP_EVAL);

            val = compare(newa, newb, add);

            ast_Cleanup(g);
            ast_Cleanup(newa);
            ast_Cleanup(newb);

            return multiplier * val;
        }

        ast_Cleanup(g);
    }

    /*Compare power bases to sort alphabetically when multiplying, and by degree when adding*/
    
    if(isoptype(b, OP_POW)) {
        temp = a;
        a = b;
        b = temp;
        multiplier *= -1;
    }
    if(isoptype(a, OP_POW)) {
        ast_t *abase, *apower;

        abase = ast_ChildGet(a, 0);
        apower = ast_ChildGet(a, 1);

        if(isoptype(b, OP_POW)) {
            ast_t *bbase, *bpower;
            
            bbase = ast_ChildGet(b, 0);
            bpower = ast_ChildGet(b, 1);

            return multiplier * (add ? -1 * compare(apower, bpower, add) : compare(abase, bbase, add));
        }

    }

    if(is_negative_for_sure(a) && !is_negative_for_sure(b))
        return multiplier * (add ? 1 : -1);
    else if(is_negative_for_sure(b) && !is_negative_for_sure(a))
        return multiplier * (add ? -1 : 1);

    /*NODE_NUMBER < NODE_SYMBOL < NODE_OPERATOR*/
    if(a->type < b->type) return multiplier * (add ? 1 : -1);
    else if(a->type > b->type) return multiplier * (add ? -1 : 1);

    switch(a->type) {
    case NODE_NUMBER:   return multiplier * mp_rat_compare(a->op.num, b->op.num);
    case NODE_SYMBOL:   return multiplier * (a->op.symbol - b->op.symbol);
    case NODE_OPERATOR: return multiplier * (optype(a) - optype(b));
    }

    return -1;
}

/*
    Order multiplication and division.
    Change XAZ to AXZ and 1+sin(X)ZA5 to 5AZsin(X)+1
    Rationalize denominators.
    Change powers to roots.
    Combine a^5b^5 to (ab)^5.

    Use only when exporting and do not plan to simplify again
    or execute any other function on ast. Messes up all other algorithms.

    Sorting for addition and multiplication is O(n^2) by insertion sort
*/
bool simplify_canonical_form(ast_t *e) {
    bool changed = false, intermediate_change;
    unsigned i;

    do {
        intermediate_change = false;

        if(isoptype(e, OP_MULT)) {
            unsigned i, j;
            bool inner_intermediate;

            /*Combine a^5b^5 to (ab)^5 */

            do {
                inner_intermediate = false;

                for(i = 0; i < ast_ChildLength(e); i++) {
                    ast_t *ichild = ast_ChildGet(e, i);

                    if(!isoptype(ichild, OP_POW))
                        continue;

                    for(j = i + 1; j < ast_ChildLength(e); j++) {
                        ast_t *jchild = ast_ChildGet(e, j);

                        if(!isoptype(jchild, OP_POW))
                            continue;

                        /*If the powers are the same*/
                        if(ast_Compare(ast_ChildGet(ichild, 1), ast_ChildGet(jchild, 1))) {

                            ast_ChildAppend(e, ast_MakeBinary(OP_POW,
                                                    ast_MakeBinary(OP_MULT,
                                                        ast_Copy(ast_ChildGet(ichild, 0)),
                                                        ast_Copy(ast_ChildGet(jchild, 0))
                                                    ),
                                                    ast_Copy(ast_ChildGet(ichild, 1))
                            ));

                            ast_Cleanup(ast_ChildRemove(e, ichild));
                            ast_Cleanup(ast_ChildRemove(e, jchild));

                            simplify(e, SIMP_COMMUTATIVE);

                            inner_intermediate = intermediate_change = changed = true;

                            break;
                        }
                    }

                    if(inner_intermediate)
                        break;
                }

            } while(inner_intermediate);
            
        } else if(isoptype(e, OP_POW)) {
            ast_t *base, *power;

            /*Change powers to roots*/

            base = ast_ChildGet(e, 0);
            power = ast_ChildGet(e, 1);

            if(isoptype(power, OP_DIV) && is_ast_int(ast_ChildGet(power, 0), 1)) {
                replace_node(e, ast_MakeBinary(OP_ROOT,
                                    ast_Copy(ast_ChildGet(power, 1)),
                                    ast_Copy(base)));
                intermediate_change = changed = true;
            }
        } else if(isoptype(e, OP_DIV)) {
            ast_t *num, *den;

            /*Rationalize denominator*/

            num = ast_ChildGet(e, 0);
            den = ast_ChildGet(e, 1);

            if(isoptype(den, OP_ROOT)) {

                replace_node(num, ast_MakeBinary(OP_MULT,
                                    ast_Copy(num),
                                    ast_Copy(den)
                                ));
                replace_node(den, ast_MakeBinary(OP_MULT,
                                    ast_Copy(den),
                                    ast_Copy(den)
                                ));
                
                simplify(num, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_EVAL | SIMP_RATIONAL | SIMP_LIKE_TERMS);
                simplify(den, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_EVAL | SIMP_RATIONAL | SIMP_LIKE_TERMS);

                intermediate_change = changed = true;
            } else if(isoptype(den, OP_MULT)) {
                ast_t *child;
                for(child = ast_ChildGet(den, 0); child != NULL; child = child->next) {
                    if(isoptype(child, OP_ROOT)) {
                        replace_node(num, ast_MakeBinary(OP_MULT,
                                            ast_Copy(num),
                                            ast_Copy(child)
                                        ));
                        replace_node(den, ast_MakeBinary(OP_MULT,
                                            ast_Copy(den),
                                            ast_Copy(child)
                                        ));
                        
                        simplify(num, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_EVAL | SIMP_RATIONAL | SIMP_LIKE_TERMS);
                        simplify(den, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_EVAL | SIMP_RATIONAL | SIMP_LIKE_TERMS);

                        intermediate_change = changed = true;
                        break;
                    }
                }
            }
        }

        /*Order addition and multiplication: C*A*D becomes A*C*D */
        for(i = 0; i < ast_ChildLength(e); i++) {
            ast_t *child = ast_ChildGet(e, i);

            if(e->type == NODE_OPERATOR && (optype(e) == OP_MULT || optype(e) == OP_ADD)) {
                unsigned j;

                for(j = 0; j < i; j++) {
                    int val;
                    ast_t *child2;

                    child2 = ast_ChildGet(e, j);
                    val = compare(child, child2, optype(e) == OP_ADD);

                    if(val < 0) {
                        ast_ChildInsert(e, ast_ChildRemoveIndex(e, i), j);
                        intermediate_change = changed = true;
                    }
                }
            }

            intermediate_change |= simplify_canonical_form(child);
            changed |= intermediate_change;

        }
    } while(intermediate_change);

    return changed;
}

/*
    Combine things like 
    AA to A^(1+1)
    A^2AB to A^(2+1)B
*/ 
bool simplify_like_terms_multiplication(ast_t *e) {
    ast_t *child = NULL;
    bool changed = false;
    
    for(child = ast_ChildGet(e, 0); child != NULL; child = child->next)
            changed |= simplify_like_terms_multiplication(child);
    
    if(isoptype(e, OP_MULT)) {
        unsigned i, j;

        for(i = 0; i < ast_ChildLength(e); i++) {
            ast_t *a, *b;

            a = ast_ChildGet(e, i);

            for(j = i + 1; j < ast_ChildLength(e); j++) {
                bool clean_power1 = false, clean_power2 = false;
                ast_t *base1 = NULL, *base2 = NULL, *power1 = NULL, *power2 = NULL;

                b = ast_ChildGet(e, j);

                if(isoptype(a, OP_POW)) {

                    base1 = ast_ChildGet(a, 0);
                    power1 = ast_ChildGet(a, 1);

                    if(isoptype(b, OP_POW)) {
                        base2 = ast_ChildGet(b, 0);
                        power2 = ast_ChildGet(b, 1);
                    } else {
                        base2 = b;
                        power2 = ast_MakeNumber(num_FromInt(1));
                        clean_power2 = true;
                    }

                } else if(isoptype(b, OP_POW)) {
                    base1 = a;
                    power1 = ast_MakeNumber(num_FromInt(1));
                    base2 = ast_ChildGet(b, 0);
                    power2 = ast_ChildGet(b, 1);

                    clean_power1 = true;
                } else {
                    base1 = a;
                    power1 = ast_MakeNumber(num_FromInt(1));
                    base2 = b;
                    power2 = ast_MakeNumber(num_FromInt(1));

                    clean_power1 = true;
                    clean_power2 = true;
                }

                if(base1 != NULL && ast_Compare(base1, base2)) {
                    ast_t *append;

                    append = ast_MakeBinary(OP_POW,
                                            ast_Copy(base1),
                                            ast_MakeBinary(OP_ADD,
                                                ast_Copy(power1),
                                                ast_Copy(power2)
                                            )
                                        );

                    ast_ChildAppend(e, append);

                    ast_Cleanup(ast_ChildRemove(e, a));
                    ast_Cleanup(ast_ChildRemove(e, b));

                    if(clean_power1) ast_Cleanup(power1);
                    if(clean_power2) ast_Cleanup(power2);

                    changed = true;

                    i--;

                    break;
                }

                if(clean_power1) ast_Cleanup(power1);
                if(clean_power2) ast_Cleanup(power2);

            }
        }

    }

    return changed;
}

/*
    Simplifies sin(9pi/4) to sin(pi/4)

    Works on sin, cos, and tan
*/
bool simplify_periodic(ast_t *e) {
    ast_t *copy;
    mp_int a, b, c, d;
    bool is_negative, changed = false;

    if(!isoptype(e, OP_SIN) && !isoptype(e, OP_COS) && !isoptype(e, OP_TAN))
        return false;

    copy = ast_MakeBinary(OP_DIV, ast_Copy(ast_ChildGet(e, 0)), ast_MakeSymbol(SYM_PI));
    simplify(copy, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL | SIMP_EVAL);

    if(!isoptype(copy, OP_DIV) && copy->type != NODE_NUMBER) {
        ast_Cleanup(copy);
        return false;
    }

    is_negative = is_negative_for_sure(copy);

    if(copy->type == NODE_NUMBER) {
        a = mp_int_alloc();
        mp_int_init_copy(a, &copy->op.num->num);
        b = mp_int_alloc();
        mp_int_init_value(b, 1);
    } else if(isoptype(copy, OP_DIV)) {
        ast_t *num, *den;

        num = ast_ChildGet(copy, 0);
        den = ast_ChildGet(copy, 1);

        if(num->type != NODE_NUMBER || den->type != NODE_NUMBER) {
            ast_Cleanup(copy);
            return false;
        }

        a = mp_int_alloc();
        mp_int_init_copy(a, &num->op.num->num);
        b = mp_int_alloc();
        mp_int_init_copy(b, &den->op.num->num);
    } else {
        ast_Cleanup(copy);
        return false;
    }

    mp_int_abs(a, a);
    mp_int_abs(b, b);

    c = mp_int_alloc();
    mp_int_init(c);

    /*c = 2b for sin and cos, c = b for tan*/
    if(isoptype(e, OP_TAN))
        mp_int_copy(b, c);
    else
        mp_int_mul_value(b, 2, c);

    d = mp_int_alloc();
    mp_int_init(d);

    /*d = a/c*/
    mp_int_div(a, c, d, NULL);

    /*if d >= 1 */
    if(mp_int_compare_value(d, 1) >= 0) {
        mp_rat new_num, new_den;

        /*a = a - cd*/
        mp_int_mul(c, d, c);
        mp_int_sub(a, c, a);

        if(is_negative)
            mp_int_neg(a, a);

        new_num = num_FromInt(1);
        new_den = num_FromInt(1);

        mp_int_copy(a, &new_num->num);
        mp_int_copy(b, &new_den->num);

        replace_node(e, ast_MakeUnary(optype(e),
                            ast_MakeBinary(OP_MULT,
                                ast_MakeSymbol(SYM_PI),
                                ast_MakeBinary(OP_DIV,
                                    ast_MakeNumber(new_num),
                                    ast_MakeNumber(new_den)
                                )
                            )));

        changed = true;
    }

    mp_int_free(a);
    mp_int_free(b);
    mp_int_free(c);
    mp_int_free(d);

    ast_Cleanup(copy);

    return changed;
}

bool simplify_identities(ast_t *e, const unsigned short flags) {
    bool changed = false;

    if(flags & SIMP_ID_GENERAL)
        while(id_ExecuteTable(e, id_general, ID_NUM_GENERAL))                   changed = true;

    if(flags & SIMP_ID_COMPLEX)
        while(id_ExecuteTable(e, id_complex, ID_NUM_COMPLEX))                   changed = true;

    if(flags & SIMP_ID_TRIG) {
        while(simplify_periodic(e))                                             changed = true;
        while(id_ExecuteTable(e, id_trig_identities, ID_NUM_TRIG_IDENTITIES))   changed = true;
    }

    if(flags & SIMP_ID_TRIG_CONSTANTS)
        while(id_ExecuteTable(e, id_trig_constants, ID_NUM_TRIG_CONSTANTS))     changed = true;

    if(flags & SIMP_ID_HYPERBOLIC)
        while(id_ExecuteTable(e, id_hyperbolic, ID_NUM_HYPERBOLIC))             changed = true;


    return changed;
}

/*
    Executes a collection of simplification functions.

    O(infinity - 1)

    Returns true if ast was changed
*/
bool simplify(ast_t *e, const unsigned short flags) {
    bool did_change = false, intermediate_change;

    do {
        intermediate_change = false;

        if(flags & SIMP_NORMALIZE)
            while(simplify_normalize(e))    intermediate_change = did_change = true;
        if(flags & SIMP_COMMUTATIVE)
            while(simplify_commutative(e))  intermediate_change = did_change = true;
        if(flags & SIMP_RATIONAL)
            while(simplify_rational(e))     intermediate_change = did_change = true;
        if(flags & SIMP_EVAL)
            while(eval(e))                  intermediate_change = did_change = true;

        /*Simplify identities. First factor the expression and simplify identities.
        Then expand the expression and simplify identities that we missed. Only factor and expand if 
        at least one id flag is set. The expression remains in the expanded state at the end of simplify().*/

        if(flags & SIMP_ID_ALL) {
            factor(e, FAC_SIMPLE_ADDITION_EVALUATEABLE | FAC_SIMPLE_ADDITION_NONEVALUATEABLE);
        }

        intermediate_change |= simplify_identities(e, flags);
        did_change |= intermediate_change;

        if(flags & SIMP_ID_ALL) {
            expand(e, EXP_DISTRIB_NUMBERS | EXP_DISTRIB_MULTIPLICATION | EXP_DISTRIB_DIVISION);
            simplify(e, SIMP_COMMUTATIVE | SIMP_EVAL | SIMP_LIKE_TERMS);
        }

        intermediate_change |= simplify_identities(e, flags);
        did_change |= intermediate_change;

        if(flags & SIMP_ID_ALL) {
            /*Undo expansion of division*/
            simplify(e, SIMP_COMMUTATIVE | SIMP_RATIONAL | SIMP_EVAL);
        }

        /*Simplify like terms*/
        if(flags & SIMP_LIKE_TERMS) {
            /*First fix 2A+B _ (A+B) to 2A+B_A_B */
            while(expand(e, EXP_DISTRIB_NUMBERS))               intermediate_change = did_change = true;
            /*Then fix 2A+B_A_B to A(2+-1)+B(1+-1)*/
            while(factor(e, FAC_SIMPLE_ADDITION_EVALUATEABLE))  intermediate_change = did_change = true;
            /*This would fix AA to A^2 if it exists*/
            while(simplify_like_terms_multiplication(e))        intermediate_change = did_change = true;
        }
        
    } while(intermediate_change);
    
    return did_change;
}