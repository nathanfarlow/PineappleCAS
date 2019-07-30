#include "cas.h"

pcas_ast_t *combine(pcas_ast_t *add, pcas_ast_t *b) {
    unsigned i, j;
    pcas_ast_t *expanded = ast_MakeOperator(OP_ADD);

    if(isoptype(b, OP_ADD)) {

        for(i = 0; i < ast_ChildLength(add); i++) {
            for(j = 0; j < ast_ChildLength(b); j++) {

                ast_ChildAppend(expanded, ast_MakeBinary(OP_MULT,
                                            ast_Copy(ast_ChildGet(add, i)),
                                            ast_Copy(ast_ChildGet(b, j))
                                        ));
            }
        }

    } else {

        for(i = 0; i < ast_ChildLength(add); i++) {
            ast_ChildAppend(expanded, ast_MakeBinary(OP_MULT,
                                        ast_Copy(ast_ChildGet(add, i)),
                                        ast_Copy(b)
                                    ));
        }
    }

    return expanded;
}

bool expand(pcas_ast_t *e, unsigned char flags) {
    unsigned i, j;

    bool did_change = false;
    bool intermediate_change = false;

    for(i = 0; i < ast_ChildLength(e); i++)
        did_change |= expand(ast_ChildGet(e, i), flags);

    do {
        intermediate_change = false;

        if(isoptype(e, OP_MULT)) {

            for(i = 0; i < ast_ChildLength(e); i++) {
                pcas_ast_t *ichild = ast_ChildGet(e, i);

                if(isoptype(ichild, OP_ADD)) {

                    for(j = 0; j < ast_ChildLength(e); j++) {
                        pcas_ast_t *jchild = ast_ChildGet(e, j);
                        if(i != j) {
                            bool should_expand;

                            /*Yes I am sure I can do this in one line in the if statement
                            but this is clearer*/

                            if(jchild->type == NODE_NUMBER)
                                should_expand = flags & EXP_DISTRIB_NUMBERS;
                            else if(isoptype(jchild, OP_ADD))
                                should_expand = flags & EXP_DISTRIB_ADDITION;
                            else
                                should_expand = flags & EXP_DISTRIB_MULTIPLICATION;

                            if(should_expand) {
                                pcas_ast_t *combined = combine(ichild, jchild);

                                ast_ChildAppend(e, combined);

                                ast_Cleanup(ast_ChildRemove(e, ichild));
                                ast_Cleanup(ast_ChildRemove(e, jchild));

                                intermediate_change = true;
                                did_change = true;
                                break;
                            }

                        }
                    }

                    if(intermediate_change)
                        break;
                }

                if(intermediate_change)
                    break;
            }

        } 

        if((flags & EXP_DISTRIB_DIVISION) && isoptype(e, OP_DIV)) {
            pcas_ast_t *num, *den;

            num = ast_ChildGet(e, 0);
            den = ast_ChildGet(e, 1);

            /*Split A/B into A * (1/B)*/
            replace_node(e, ast_MakeBinary(OP_MULT, ast_Copy(num), ast_MakeBinary(OP_DIV, ast_MakeNumber(num_FromInt(1)), ast_Copy(den))));

            /*This could be dangerous, but we'll cross that bridge when we get there.*/
            expand(e, EXP_DISTRIB_NUMBERS | EXP_DISTRIB_ADDITION | EXP_DISTRIB_MULTIPLICATION | EXP_DISTRIB_ADDITION);

            intermediate_change = true;
            did_change = true;
            continue;
        } else if(isoptype(e, OP_POW)) {
            pcas_ast_t *exponent, *div_node;

            div_node = ast_ChildGet(e, 0);
            exponent = ast_ChildGet(e, 1);

            if(isoptype(div_node, OP_DIV)) {
                pcas_ast_t *new_num, *new_den, *new_div;
                new_num = ast_MakeBinary(OP_POW, ast_Copy(ast_ChildGet(div_node, 0)), ast_Copy(exponent));
                new_den = ast_MakeBinary(OP_POW, ast_Copy(ast_ChildGet(div_node, 1)), ast_Copy(exponent));
                new_div = ast_MakeBinary(OP_DIV, new_num, new_den);

                replace_node(e, new_div);

                intermediate_change = true;
                did_change = true;
                continue;
            }
        }

        if((flags & EXP_EXPAND_POWERS) && isoptype(e, OP_POW)) {
            mp_int val;
            pcas_ast_t *base, *power, *replacement;

            base = ast_ChildGet(e, 0);
            power = ast_ChildGet(e, 1);

            if(isoptype(base, OP_ADD) && mp_rat_is_integer(power->op.num)) {
                val = &power->op.num->num;
                replacement = ast_MakeOperator(OP_MULT);

                while(mp_int_compare_zero(val) > 0) {

                    ast_ChildAppend(replacement, ast_Copy(base));

                    mp_int_sub_value(val, 1, val);
                }

                replace_node(e, replacement);

                intermediate_change = true;
                did_change = true;
            }

        }

    } while(intermediate_change);

    return did_change;
}