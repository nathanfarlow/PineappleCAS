#include "cas.h"
#include "simplify.h"

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
#include "../dbg.h"
bool expand(ast_t *e, bool expand_powers) {
    bool changed = true;

    do {
        unsigned i, j;

        changed = false;

        if(isoptype(e, OP_MULT)) {

            for(i = 0; i < ast_ChildLength(e); i++) {
                ast_t *ichild = ast_ChildGet(e, i);

                if(isoptype(ichild, OP_ADD)) {

                    for(j = 0; j < ast_ChildLength(e); j++) {
                        ast_t *jchild = ast_ChildGet(e, j);
                        if(i != j) {
                            ast_t *combined = combine(ichild, jchild);

                            ast_ChildAppend(e, combined);

                            ast_Cleanup(ast_ChildRemove(e, ichild));
                            ast_Cleanup(ast_ChildRemove(e, jchild));
                            
                            simplify(e);

                            changed = true;
                            break;
                        }
                    }

                    if(changed)
                        break;
                }
            }

        } else if(expand_powers && isoptype(e, OP_POW)) {
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

                changed = true;
                continue;
            }
            
        }

        for(i = 0; i < ast_ChildLength(e); i++)
            changed |= expand(ast_ChildGet(e, i), expand_powers);
    } while(changed);

    simplify(e);

    return changed;
}