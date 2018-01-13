#include "cas.h"

#include <stdbool.h>
#include "../debug.h"

#define is_type_communative(type) (type == OP_ADD || type == OP_MULT)
#define is_type_operator(type) (type >= OP_ADD && type <= OP_LOG)
#define is_type_function(type) (type >= OP_INT && type <= OP_TANH_INV)

/*
Scrambles the order of the parameters, but that's fine
because we're going to put them in canonical form anyway.

Returns true if the structure of the ast was changed
*/
static bool fix_communative(ast_t *node) {
	unsigned i;
	bool changed = false;

	if(node->type != NODE_OPERATOR || !is_type_communative(node->op.operator.type))
		return false;

	for(i = 0; i < ast_ChildLength(node); i++) {

		ast_t *child = ast_ChildGet(node, i);

		if(child->type == NODE_OPERATOR && child->op.operator.type == node->op.operator.type) {
			ast_ChildAppend(node, child->op.operator.base);

			ast_ChildRemove(node, child);
			child->op.operator.base = NULL;
			ast_Cleanup(child);

			changed = true;
			i--;
		} else {
			changed |= fix_communative(child);
		}
	}

	return changed;
}

bool simplify(ast_t *e) {
	bool changed = false;

	if(e->type != NODE_OPERATOR)
		return false;

	while(fix_communative(e))
		changed = true;

	return changed;
}