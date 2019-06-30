#include "identities.h"

/*For calloc*/
#include <stdlib.h>
/*For strlen*/
#include <string.h>

#include "cas.h"
#include "../dbg.h"

id_t id_inverses[ID_NUM_INVERSES] = {
	{"A^logb(B,A", "B"},
	{"logb(A^B,A", "B"},
	{"logb(A,A", "1"},

	{"(ArootB)^A", "B"}, /*Todo ignores negatives*/
	{"Aroot(B^A)", "B"},

	{"sin(asin(X", "X"},
	{"asin(sin(X", "X"},
	{"cos(acos(X", "X"},
	{"acos(cos(X", "X"},
	{"tan(atan(X", "X"},
	{"atan(tan(X", "X"},

	{"sinh(asinh(X", "X"},
	{"asinh(sinh(X", "X"},
	{"cosh(acosh(X", "X"},
	{"tanh(atanh(X", "X"},
	{"atanh(tanh(X", "X"}
};

id_t id_trig_identities[ID_NUM_TRIG_IDENTITIES] = {
    {"sin(X)/cos(X", "tan(X"},
	{"cos(X)/sin(X", "1/tan(X"},

	{"sin(pi/2_X", "cos(X"},
	{"cos(pi/2_X", "sin(X"},

	{"sin(X+2piN", "sin(X"},
	{"sin(X+2pi", "sin(X"},
	{"cos(X+2piN", "cos(X"},
	{"cos(X+2pi", "cos(X"},
	{"tan(X+piN", "tan(X"},
	{"tan(X+pi", "tan(X"},

	{"sin(-X", "-sin(X"},
	{"cos(-X", "cos(X"},
	{"tan(-X", "-tan(X"},

	/*Double angle identities*/
	{"2sin(X)cos(X", "sin(2X"},
	{"cos(X)^2_sin(X)^2", "cos(2X"},
	{"2cos(X)^2_1", "cos(2X"},
	{"1_2sin(X)^2", "cos(2X"},

    {"sin(X)^2+cos(X)^2", "1"}
};

id_t id_trig_constants[ID_NUM_TRIG_CONSTANTS] = {
	{"sin(0", "0"},
	{"sin(pi/6", "1/2"},
    {"sin(pi/4", "sqrt(2)/2"},
	{"sin(pi/3", "sqrt(3)/2"},
	{"sin(pi/2", "1"},
	{"sin(2pi/3", "sqrt(3)/2"},
	{"sin(3pi/4", "sqrt(2)/2"},
	{"sin(5pi/6", "1/2"},
	{"sin(pi", "0"},
	{"sin(7pi/6", "-1/2"},
	{"sin(5pi/4", "-sqrt(2)/2"},
	{"sin(4pi/3", "-sqrt(3)/2"},
	{"sin(3pi/2", "-1"},
	{"sin(5pi/3", "-sqrt(3)/2"},
	{"sin(7pi/4", "-sqrt(2)/2"},
	{"sin(11pi/6", "-1/2"},

	{"cos(0", "1"},
	{"cos(pi/6", "sqrt(3)/2"},
    {"cos(pi/4", "sqrt(2)/2"},
	{"cos(pi/3", "1/2"},
	{"cos(pi/2", "0"},
	{"cos(2pi/3", "-1/2"},
	{"cos(3pi/4", "-sqrt(2)/2"},
	{"cos(5pi/6", "-sqrt(3)/2"},
	{"cos(pi", "-1"},
	{"cos(7pi/6", "-sqrt(3)/2"},
	{"cos(5pi/4", "-sqrt(2)/2"},
	{"cos(4pi/3", "-1/2"},
	{"cos(3pi/2", "0"},
	{"cos(5pi/3", "1/2"},
	{"cos(7pi/4", "sqrt(2)/2"},
	{"cos(11pi/6", "sqrt(3)/2"},

	{"tan(0", "0"},
	{"tan(pi/6", "sqrt(3)/3"},
    {"tan(pi/4", "1"},
	{"tan(pi/3", "sqrt(3)"},
	/*{"tan(pi/2", "0"},*/
	{"tan(2pi/3", "-sqrt(3"},
	{"tan(3pi/4", "-1"},
	{"tan(5pi/6", "-sqrt(3)/3"},
	{"tan(pi", "0"},
	{"tan(7pi/6", "sqrt(3)/3"},
	{"tan(5pi/4", "1"},
	{"tan(4pi/3", "sqrt(3"},
	/*{"tan(3pi/2", "0"},*/
	{"tan(5pi/3", "-sqrt(3"},
	{"tan(7pi/4", "-1"},
	{"tan(11pi/6", "-sqrt(3)/3"},
};

id_t id_hyperbolic[ID_NUM_HYPERBOLIC] = {
	{"sinh(X)/cosh(X", "tanh(X"},
	{"cosh(X)^2_sinh(X)^2", "1"}
};

typedef ast_t** Dictionary;

#define dict_Get(dict, ast_symbol) 	dict[ast_symbol->op.symbol - 'A']

void dict_Copy(Dictionary dest, Dictionary source) {
	unsigned i;
	for(i = 0; i < AMOUNT_SYMBOLS; i++) {
		if(source[i] != NULL)
			dest[i] = ast_Copy(source[i]);
		else
			dest[i] = NULL;
	}
}

void dict_Write(Dictionary dest, Dictionary source) {
	memcpy(dest, source, AMOUNT_SYMBOLS * sizeof(ast_t*));
}

void dict_Cleanup(Dictionary dict) {
	unsigned i;
	for(i = 0; i < AMOUNT_SYMBOLS; i++) {
		if(dict[i] != NULL)
			ast_Cleanup(dict[i]);
		dict[i] = NULL;
	}
}

/*Simplifies 2N, 4 to N, 2 to correctly set N*/
void fix_number(ast_t *id, ast_t *e) {
	ast_t *child;

	if(!isoptype(id, OP_MULT))
		return;

	if(is_ast_int(e, 0))
		return;

	for(child = ast_ChildGet(id, 0); child != NULL; child = child->next) {
		if(child->type == NODE_NUMBER) {
			replace_node(e, ast_MakeBinary(OP_DIV, ast_Copy(e), ast_Copy(child)));
			replace_node(id, ast_MakeBinary(OP_DIV, ast_Copy(id), ast_Copy(child)));

			simplify(e, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL | SIMP_EVAL);
			simplify(id, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL | SIMP_EVAL);

			return;
		}
	}
}

bool matches(ast_t *id, ast_t *e, Dictionary dict) {

	if(id->type == NODE_SYMBOL && id->op.symbol < SYM_PI) {
		if(id->op.symbol == 'N') {
			/*Only integers allowed*/
			if(!(e->type == NODE_NUMBER && mp_rat_is_integer(e->op.num)))
				return false;
		}

		/*Check if dictionary does not yet have value*/
		if(dict_Get(dict, id) == NULL) {
			/*No value exists in dictionary, this node claims it*/
			dict_Get(dict, id) = ast_Copy(e);
			return true;
		} else {
			/*Value already exists, we only match if we are the same*/
			return ast_Compare(dict_Get(dict, id), e);
		}
	} else if(id->type == NODE_OPERATOR) {
		unsigned i, j;

		/*Make a copy of the dictionary in case the children do not match
		We do not fill the dictionary with bad values*/
		ast_t *dict_copy[AMOUNT_SYMBOLS];

		dict_Copy(dict_copy, dict);

		if(is_op_commutative(optype(id))) {
			/*Order does not matter*/
 
			bool matched, *matched_e_children, *matched_id_children;

			ast_t *id_copy, *e_copy;

			id_copy = ast_Copy(id);
			e_copy = ast_Copy(e);

			fix_number(id_copy, e_copy);

			/*If simplified 2N, 4 to N, 2 */
			if(id_copy->type != NODE_OPERATOR && e_copy->type != NODE_OPERATOR) {
				matched = matches(id_copy, e_copy, dict_copy);

				if(matched) {
					dict_Cleanup(dict);
					dict_Write(dict, dict_copy);
				} else {
					dict_Cleanup(dict_copy);
				}

				ast_Cleanup(id_copy);
				ast_Cleanup(e_copy);

				return matched;
			}
			/*If simplified 4N, 2 to 2N, 1 */ 
			else if(id_copy->type != e_copy->type) {
				dict_Cleanup(dict_copy);
				ast_Cleanup(id_copy);
				ast_Cleanup(e_copy);
				return false;
			}

			/*At this point, id_copy and e_copy are both the same either addition
			or multiplication nodes and we can continue normally*/

			if(optype(e) != optype(id) || ast_ChildLength(id_copy) > ast_ChildLength(e_copy)) {
				dict_Cleanup(dict_copy);
				ast_Cleanup(id_copy);
				ast_Cleanup(e_copy);
				return false;
			}

			matched_e_children = calloc(ast_ChildLength(e_copy), sizeof(bool));
			matched_id_children = calloc(ast_ChildLength(id_copy), sizeof(bool));

			do {
				matched = false;

				for(i = 0; i < ast_ChildLength(e_copy); i++) {
					ast_t *e_child = ast_ChildGet(e_copy, i);

					if(matched_e_children[i])
						continue;

					for(j = 0; j < ast_ChildLength(id_copy); j++) {
						ast_t *dict_copy_copy[AMOUNT_SYMBOLS];
						ast_t *id_child = ast_ChildGet(id_copy, j);

						dict_Copy(dict_copy_copy, dict_copy);

						if(!matched_id_children[j]) {
							
							if(matches(id_child, e_child, dict_copy_copy)) {
								matched_e_children[i] = true;
								matched_id_children[j] = true;
								matched = true;

								dict_Cleanup(dict_copy);
								dict_Write(dict_copy, dict_copy_copy);

								break;
							}
						}

						dict_Cleanup(dict_copy_copy);

					}

					if(matched)
						break;

				}

			} while(matched);

			/*Check if we have matched every id child. We don't have to match
            every e child due to commutative nature.*/
			matched = true;
			for(i = 0; i < ast_ChildLength(id_copy); i++)
				matched &= matched_id_children[i];

			free(matched_e_children);
			free(matched_id_children);

			ast_Cleanup(e_copy);
			ast_Cleanup(id_copy);

			if(matched) {
				dict_Cleanup(dict);
				dict_Write(dict, dict_copy);
			} else {
				dict_Cleanup(dict_copy);
			}

			return matched;
			
		} else {
			/*Order does matter*/

			if(e->type != NODE_OPERATOR) {
				dict_Cleanup(dict_copy);
				return false;
			}

			if(optype(e) != optype(id)) {
				dict_Cleanup(dict_copy);				
				return false;
			}

			for(i = 0; i < ast_ChildLength(e); i++) {
				ast_t *e_child = ast_ChildGet(e, i);
				ast_t *id_child = ast_ChildGet(id, i);
				if(!matches(id_child, e_child, dict_copy)) {
					dict_Cleanup(dict_copy);
					return false;
				}
			}

			/*All children are matching. Write the copy to the actual dict*/
			dict_Cleanup(dict);
			dict_Write(dict, dict_copy);
			return true;
		}

	}

	/*Compare numbers or pi, e constants */
	return ast_Compare(e, id);
}

/*Fills in id->to with discovered values from dictionary */
void fill(ast_t *to, Dictionary dict) {

	if(to->type == NODE_SYMBOL) {
		if(dict_Get(dict, to) != NULL)
			replace_node(to, ast_Copy(dict_Get(dict, to)));
	} else if(to->type == NODE_OPERATOR) {
		ast_t *child;
		for(child = ast_ChildGet(to, 0); child != NULL; child = child->next) {
			fill(child, dict);
		}
	}

}

/*
	Requires that constants are already evaluated.
*/
bool id_Execute(ast_t *e, id_t *id) {
    ast_t *child;
	ast_t *dict[AMOUNT_SYMBOLS] = {0};
    bool changed = false;

    if(id->from == NULL || id->to == NULL) {
        if(!id_Load(id)) {
            LOG(("Could not parse identity. from=%s to=%s", id->from_text, id->to_text));
            return false;
        }
    }

	if(e->type == NODE_OPERATOR) {
		for(child = ast_ChildGet(e, 0); child != NULL; child = child->next)
			changed |= id_Execute(child, id);
	}

    if(matches(id->from, e, dict)) {
		ast_t *to = ast_Copy(id->to);
			
		fill(to, dict);

		if(is_op_commutative(optype(e))) {
			ast_t *found, *replacement = NULL;

			found = ast_Copy(id->from);
			fill(found, dict);

            /*If we found an identity among the nodes of a commutative operator.
            For example, sin(X)^2 + cos(X)^2 + A changes to 1 + sin(X)^2 + cos(X)^2 _ (sin(X)^2 + cos(X)^2) + A
            and is later simplified to just A + 1 */
            if(ast_ChildLength(e) != ast_ChildLength(found)) {

                if(isoptype(e, OP_MULT)) {
                	replacement = ast_MakeBinary(OP_MULT,
                                                    ast_MakeBinary(OP_DIV,
                                                        ast_Copy(e),
                                                        found	
                                                    ),
                                                    to
                                                );
                } else if(isoptype(e, OP_ADD)) {
                    replacement = ast_MakeBinary(OP_ADD,
                                                    ast_MakeBinary(OP_ADD,
                                                        ast_Copy(e),
                                                        ast_MakeBinary(OP_MULT,
                                                            found,
                                                            ast_MakeNumber(num_FromInt(-1))
                                                        )
                                                    ),
                                                    to
                                                );
                }

				simplify(replacement, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL | SIMP_EVAL | SIMP_LIKE_TERMS);
				replace_node(e, replacement);
            } else {
				ast_Cleanup(found);
				replace_node(e, to);
			}
            
		} else {
			replace_node(e, to);
		}
			
        changed = true;
	}

    dict_Cleanup(dict);

    return changed;
}

bool id_Load(id_t *id) {
	error_t err;

	id->from = parse((uint8_t*)id->from_text, strlen(id->from_text), str_table, &err);

	if(err != E_SUCCESS)
		return false;

	id->to = parse((uint8_t*)id->to_text, strlen(id->to_text), str_table, &err);

	if(err != E_SUCCESS)
		return false;

	if(id->from != NULL && id->to != NULL) {
		/*Assumes that from and to are already simplified. This just puts it into a form we can compare*/
		simplify(id->from, SIMP_NORMALIZE | SIMP_COMMUTATIVE);
		simplify(id->to, SIMP_NORMALIZE | SIMP_COMMUTATIVE);
		return true;
	}

	return false;
}

void id_Unload(id_t *id) {
    if(id->from != NULL)
        ast_Cleanup(id->from);
    id->from = NULL;
    
    if(id->to != NULL)
	    ast_Cleanup(id->to);
    id->to = NULL;
}

bool id_ExecuteTable(ast_t *e, id_t *table, unsigned table_len) {
    unsigned i;
	bool changed = false;

	for(i = 0; i < table_len; i++) {
		changed |= id_Execute(e, &table[i]);

		/*Break if changed to save time on the calculator becaus chances are good
		that after an identity is applied, we do not need to go through the rest.*/
		if(changed)
			break;
	}

	return changed;
}

void id_UnloadTable(id_t *table, unsigned table_len) {
	unsigned i;
	for(i = 0; i < table_len; i++)
		id_Unload(&table[i]);
}

void id_UnloadAll() {
	id_UnloadTable(id_inverses, ID_NUM_INVERSES);
	id_UnloadTable(id_trig_identities, ID_NUM_TRIG_IDENTITIES);
	id_UnloadTable(id_trig_constants, ID_NUM_TRIG_CONSTANTS);
	id_UnloadTable(id_hyperbolic, ID_NUM_HYPERBOLIC);
}