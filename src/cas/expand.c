#include "cas.h"

/*In simplify.c*/
bool simplify_like_terms(ast_t *e);

ast_t *combine(ast_t *add, ast_t *b) {
    unsigned i, j;
    ast_t *expanded = ast_MakeOperator(OP_ADD);

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


bool expand(ast_t *e, const unsigned char flags) {
    unsigned i, j;

    bool did_change = false;
    bool intermediate_change = false;

    for(i = 0; i < ast_ChildLength(e); i++)
        did_change |= expand(ast_ChildGet(e, i), flags);

    do {
        intermediate_change = false;

        if(isoptype(e, OP_MULT)) {

            for(i = 0; i < ast_ChildLength(e); i++) {
                ast_t *ichild = ast_ChildGet(e, i);

                if(isoptype(ichild, OP_ADD)) {

                    for(j = 0; j < ast_ChildLength(e); j++) {
                        ast_t *jchild = ast_ChildGet(e, j);
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
                                ast_t *combined = combine(ichild, jchild);

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
            }
            
        } else if((flags & EXP_EXPAND_POWERS) && isoptype(e, OP_POW)) {
            mp_int val;
            ast_t *base, *power, *replacement;

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
                continue;
            }
            
        }

    } while(intermediate_change);

    return did_change;
}