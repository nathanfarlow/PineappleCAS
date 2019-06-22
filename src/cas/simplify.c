#include "cas.h"

#include <stdbool.h>


/*
    Flattens multiplication and addition nodes

    Scrambles the order of the parameters, but that's fine
    because we're going to put them in canonical form anyway.
*/
static bool simplify_commutative(ast_t *e) {
    unsigned i;
    bool changed = false;

    if(e->type != NODE_OPERATOR)
        return false;

    if(ast_ChildLength(e) == 1
        && (optype(e) == OP_MULT || optype(e) == OP_ADD)) {
        replace_node(e, opbase(e));
        return simplify_commutative(e);
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

/*
    Simplifies rational functions.
    A/B/C/D becomes A/(BCD)
*/
static bool simplify_rational(ast_t *e) {
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
            }

        } else {
            changed |= simplify_rational(child);
        }
    }

    return changed;
}


/*Reduces every number node. Changes all roots to powers.*/
static bool simplify_reduce(ast_t *e) {
    ast_t *child;

    if(e->type == NODE_OPERATOR) {
        for(child = opbase(e); child != NULL; child = child->next)
            simplify_reduce(child);

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
        }
    } else if(e->type == NODE_NUMBER && !mp_rat_is_integer(e->op.num)) {
        mp_rat num, den;

        num = num_FromInt(1);
        den = num_FromInt(1);

        mp_rat_reduce(e->op.num);

        mp_int_copy(&e->op.num->num, &num->num);
        mp_int_copy(&e->op.num->den, &den->num);

        replace_node(e, ast_MakeBinary(OP_DIV, ast_MakeNumber(num), ast_MakeNumber(den)));
    }
    
    /*Just return false because this simplification is done in one step*/
    return false;
}

/*returns negative if a < b, 0 if a=b, positive if a > b in terms of sorting order*/
static int compare(ast_t *a, ast_t *b) {

    /*NODE_NUMBER < NODE_SYMBOL < NODE_OPERATOR*/
    if(a->type < b->type) return -1;
    else if(a->type > b->type) return 1;

    switch(a->type) {
    case NODE_NUMBER:   return mp_rat_compare(a->op.num, b->op.num);
    case NODE_SYMBOL:   return a->op.symbol - b->op.symbol;
    case NODE_OPERATOR: return optype(a) - optype(b);
    }

    return -1;
}

/*
    Order multiplication and division
    Sorting for addition and multiplication is O(n^2) by insertion sort
*/
static bool simplify_canonical_form(ast_t *e) {
    bool changed = false;
    unsigned i;

    if(isoptype(e, OP_POW)) {
        ast_t *base, *power;

        base = ast_ChildGet(e, 0);
        power = ast_ChildGet(e, 1);

        if(isoptype(power, OP_DIV) && is_ast_int(ast_ChildGet(power, 0), 1)) {
                replace_node(e, ast_MakeBinary(OP_ROOT,
                                                ast_Copy(ast_ChildGet(power, 1)),
                                                ast_Copy(base)));
                changed = true;
        }
    }

    for(i = 0; i < ast_ChildLength(e); i++) {
        ast_t *child = ast_ChildGet(e, i);

        if(e->type == NODE_OPERATOR && (optype(e) == OP_MULT || optype(e) == OP_ADD)) {
            unsigned j;

            for(j = 0; j < i; j++) {
                int val;
                ast_t *child2;

                child2 = ast_ChildGet(e, j);
                val = compare(child, child2);

                /*Swap the order if adding, because I like it that way better*/
                /*If there is an actual official order we can change this*/
                if(isoptype(e, OP_ADD) && (child->type == NODE_OPERATOR || child2->type == NODE_OPERATOR))
                    val *= -1;

                if(val < 0) {
                    ast_ChildInsert(e, ast_ChildRemoveIndex(e, i), j);
                    changed = true;
                }
            }
        }

        changed |= simplify_canonical_form(child);

    }

    return changed;
}

bool factor_addition(ast_t *e) {
    return false;
}

/*
    Combine things like 
    AA to A^2
    A+A to 2A
    A + 2A to 3A

    This function also will factor expressions such that
    A + AB becomes A(1 + B)
*/ 
static bool simplify_like_terms(ast_t *e) {
    bool changed = false;



    return changed;
}

/*
    Executes a collection of simplification functions.

    O(infinity - 1)

    Returns true if ast was changed
*/
bool simplify(ast_t *e) {
    bool changed;

    do {
        changed = false;
        while(simplify_reduce(e))       changed = true;
        while(simplify_commutative(e))  changed = true;
        while(simplify_rational(e))     changed = true;
        while(simplify_like_terms(e))   changed = true;
        while(eval(e))                  changed = true;
    } while(changed);

    while(simplify_canonical_form(e));
    
    return changed;
}