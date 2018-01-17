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

static bool _simplify(ast_t *e) {
    bool changed = false;

    while(simplify_communative(e))
        changed = true;

    while(simplify_rational(e))
        changed = true;

    return changed;
}

bool simplify(ast_t *e) {
    bool changed = false;

    if(e->type != NODE_OPERATOR)
        return false;

    while(_simplify(e))
        changed = true;

    return changed;
}