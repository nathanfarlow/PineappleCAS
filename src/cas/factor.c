#include "cas.h"

/*Handles gcd for AB, BC gcd = B*/
static ast_t *gcd_mult(ast_t *mult, ast_t *b) {
    unsigned i;
    ast_t *current_gcd, *copy;

    copy = ast_Copy(b);

    current_gcd = ast_MakeOperator(OP_MULT);

    for(i = 0; i < ast_ChildLength(mult); i++) {
        ast_t *inner_gcd;
        ast_t *child = ast_ChildGet(mult, i);

        inner_gcd = gcd(child, copy);

        ast_ChildAppend(current_gcd, inner_gcd);

        replace_node(copy, ast_MakeBinary(OP_DIV, ast_Copy(copy), ast_Copy(inner_gcd)));

        simplify(copy);
    }

    ast_Cleanup(copy);

    return current_gcd;
}

/*Handles gcd for A + AB, A gcd = A*/
static ast_t *gcd_add(ast_t *add, ast_t *b) {
    unsigned i;
    ast_t *current_gcd;

    current_gcd = gcd(ast_ChildGet(add, 0), b);

    for(i = 1; i < ast_ChildLength(add); i++) {
        ast_t *temp_gcd, *child;
        
        child = ast_ChildGet(add, i);
        temp_gcd = gcd(current_gcd, child);

        ast_Cleanup(current_gcd);
        current_gcd = temp_gcd;
    }

    return current_gcd;
}

/*gcd of both bases, raised to smallest power*/
static ast_t *gcd_pow(ast_t *pow, ast_t *b) {
    ast_t *base1, *power1;

    base1 = ast_ChildGet(pow, 0);
    power1 = ast_ChildGet(pow, 1);

    /*We purposefully ignore A^X, A^Y and only yield when
    we can determine the numerical powers*/

    if(isoptype(b, OP_POW)) {
        ast_t *power2, *base2;

        power1 = ast_ChildGet(pow, 1);
        base2 = ast_ChildGet(b, 0);
        power2 = ast_ChildGet(b, 1);

        if(power1->type == NODE_NUMBER && power2->type == NODE_NUMBER) {
            bool use_first;
            ast_t *current_gcd;

            use_first = mp_rat_compare(power1->op.num, power2->op.num) < 0;

            current_gcd = gcd(base1, base2);

            return ast_MakeBinary(OP_POW, current_gcd, use_first ? ast_Copy(power1) : ast_Copy(power2));
        }

    } else if(power1->type == NODE_NUMBER && mp_rat_compare_value(power1->op.num, 1, 1) >= 0 && ast_Compare(base1, b)) {
        return ast_Copy(b);
    }

    return ast_MakeNumber(num_FromInt(1));
}

ast_t *gcd(ast_t *a, ast_t *b) {
    ast_t *ret = NULL;

    if(ast_Compare(a, b))
        return ast_Copy(a);

    if(a->type == NODE_NUMBER && b->type == NODE_NUMBER) {
        if(mp_rat_is_integer(a->op.num) && mp_rat_is_integer(b->op.num)) {
            mp_rat gcd;
            gcd = num_FromInt(1);

            mp_int_gcd(&a->op.num->num, &b->op.num->num, &gcd->num);

            return ast_MakeNumber(gcd);
        }
    }

    if(isoptype(a, OP_ADD))
        ret = gcd_add(a, b);
    else if(isoptype(b, OP_ADD))
        ret = gcd_add(b, a);
    else if(isoptype(a, OP_MULT))
        ret = gcd_mult(a, b);
    else if(isoptype(b, OP_MULT))
        ret = gcd_mult(b, a);
    else if(isoptype(a, OP_POW))
        ret = gcd_pow(a, b);
    else if(isoptype(b, OP_POW))
        ret = gcd_pow(b, a);

    if(ret != NULL) {
        simplify(ret);
        return ret;
    }

    return ast_MakeNumber(num_FromInt(1));
}

void factor_addition(ast_t *e) {
    ast_t *child;

    for(child = ast_ChildGet(e, 0); child != NULL; child = child->next)
        factor_addition(child);

    if(isoptype(e, OP_ADD)) {
        unsigned i, j;
        ast_t *g;

        for(i = 0; i < ast_ChildLength(e); i++) {
            ast_t *a = ast_ChildGet(e, i);

            for(j = i + 1; j < ast_ChildLength(e); j++) {
                ast_t *b = ast_ChildGet(e, j);

                g = gcd(a, b);

                if(!is_ast_int(g, 1)) {
                    ast_t *append;

                    append = ast_MakeBinary(OP_MULT,
                                    ast_Copy(g),
                                    ast_MakeBinary(OP_ADD,
                                        ast_MakeBinary(OP_DIV,
                                            ast_Copy(a),
                                            ast_Copy(g)
                                        ),
                                        ast_MakeBinary(OP_DIV,
                                            ast_Copy(b),
                                            ast_Copy(g)
                                        )
                                    )
                                );

                    simplify(append);
                    ast_ChildAppend(e, append);

                    ast_Cleanup(ast_ChildRemove(e, a));
                    ast_Cleanup(ast_ChildRemove(e, b));

                    ast_Cleanup(g);
                    break;
                } else {
                    ast_Cleanup(g);
                }

            }
        }
    }
}