#include "ast.h"

ast_t *ast_MakeNumber(num_t *num) {
	ast_t *ret = malloc(sizeof(ast_t));
	ret->type = NODE_NUMBER;
	ret->next = NULL;

	ret->op.number = num;

	return ret;
}

ast_t *ast_MakeSymbol(char symbol) {
	ast_t *ret = malloc(sizeof(ast_t));
	ret->type = NODE_SYMBOL;
	ret->next = NULL;

	ret->op.symbol = symbol;

	return ret;
}

ast_t *ast_MakeOperator(OperatorType type) {
	ast_t *ret = malloc(sizeof(ast_t));
	ret->type = NODE_OPERATOR;
	ret->next = NULL;

	ret->op.operator.type = type;
	ret->op.operator.base = NULL;

	return ret;
}

ast_t *ast_MakeUnary(OperatorType type, ast_t *operand) {
	ast_t *ret = ast_MakeOperator(type);
	ast_ChildAppend(ret, operand);
	return ret;
}

ast_t *ast_MakeBinary(OperatorType type, ast_t *left, ast_t *right) {
	ast_t *ret = ast_MakeOperator(type);
	ast_ChildAppend(ret, left);
	ast_ChildAppend(ret, right);
	return ret;
}

void ast_Cleanup(ast_t *e) {
	if(e == NULL)
		return;

	switch(e->type) {
	case NODE_NUMBER:
		num_Cleanup(e->op.number);
		break;
	case NODE_SYMBOL:
		break;
	case NODE_OPERATOR: {
		//Free each node in the list
		ast_t *current = e->op.operator.base;
		while(current != NULL) {
			ast_t *next = current->next;
			ast_Cleanup(current);
			current = next;
		}
		break;
	}
	}

	free(e);
}

ast_error ast_ChildAppend(ast_t *parent, ast_t *child) {
	ast_t *last;

	if(parent->type != NODE_OPERATOR)
		return E_AST_NOT_ALLOWED;

	last = ast_ChildGetLast(parent);

	if(last == NULL)
		parent->op.operator.base = child;
	else
		last->next = child;
	
	return E_AST_SUCCESS;
}

ast_t *ast_ChildGet(ast_t *parent, LSIZE index) {
	LSIZE i;
	ast_t *current;

	if(parent->type != NODE_OPERATOR)
		return NULL;

	current = parent->op.operator.base;

	for(i = 0; i <= index && current != NULL; i++) {
		if(i == index)
			return current;
		current = current->next;
	}

	return 0;
}

ast_t *ast_ChildGetLast(ast_t *parent) {
	if(parent->type != NODE_OPERATOR)
		return NULL;

	if(parent->op.operator.base == NULL) {
		return NULL;
	} else {
		ast_t *current;
		for(current = parent->op.operator.base; current->next != NULL; current = current->next);
		return current;
	}

	return NULL;
}

ast_error ast_ChildInsert(ast_t *parent, ast_t *child, LSIZE index) {
	LSIZE i;
	ast_t *current;

	if(parent->type != NODE_OPERATOR)
		return E_AST_NOT_ALLOWED;

	if(index == 0) {
		if(parent->op.operator.base == NULL)
			parent->op.operator.base = child;
		else {
			child->next = parent->op.operator.base;
			parent->op.operator.base = child;
		}

		return E_AST_SUCCESS;
	}

	i = 1;
	current = parent->op.operator.base;

	while(current != NULL) {

		if(i == index) {
			ast_t *temp = current->next;
			current->next = child;
			child->next = temp;

			return E_AST_SUCCESS;
		}

		current = current->next;
		i++;
	}

	return E_AST_OUT_OF_BOUNDS;
}

ast_t *ast_ChildRemove(ast_t *parent, ast_t *child) {
	if(parent->type != NODE_OPERATOR)
		return NULL;
	return ast_ChildRemoveIndex(parent, ast_ChildIndexOf(parent, child));
}

LSIZE ast_ChildIndexOf(ast_t *parent, ast_t *child) {
	LSIZE i;
	ast_t *current;

	if(parent->type != NODE_OPERATOR)
		return -1;

	i = 0;

	for(current = parent->op.operator.base; current != NULL; current = current->next) {
		if(current == child)
			return i;
		i++;
	}

	return -1;
}

ast_t *ast_ChildRemoveIndex(ast_t *parent, LSIZE index) {
	LSIZE i;
	ast_t *current;

	if(parent->type != NODE_OPERATOR)
		return NULL;

	if(index == 0) {
		if(parent->op.operator.base == NULL)
			return NULL;
		ast_t *temp = parent->op.operator.base;
		parent->op.operator.base = parent->op.operator.base->next;
		return temp;
	}

	i = 1;
	current = parent->op.operator.base;

	while(current != NULL) {

		if(i == index) {
			ast_t *temp = current->next;
			current->next = temp == NULL ? NULL : temp->next;
			return temp;
		}

		current = current->next;
		i++;
	}

	return NULL;
}

LSIZE ast_ChildLength(ast_t *parent) {
	LSIZE i;
	ast_t *current;

	if(parent->type != NODE_OPERATOR)
		return 0;

	i = 0;
	for(current = parent->op.operator.base; current != NULL; current = current->next)
		i++;

	return i;
}