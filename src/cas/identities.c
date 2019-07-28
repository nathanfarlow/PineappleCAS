#include "identities.h"

/*For calloc*/
#include <stdlib.h>
/*For strlen*/
#include <string.h>

#include "cas.h"
#include "../dbg.h"

/*
    IDENTITY RULES:
    N is reserved for integers.

    I and J are reserved for real numbers.
*/
pcas_id_t id_general[ID_NUM_GENERAL] = {
    /*logb(value, base)*/
    
    /*This identity is hardcoded in eval.c so that it executes before
    the power node is evaluated with two numerical values*/
    /*{"logb(X^D,B", "Dlogb(X,B"}*/

    {"logb(X,B)+logb(Y,B)+C", "logb(XY,B)+C"},
    {"logb(X,B)_logb(Y,B)+C", "logb(X/Y,B)+C"},

    {"A^(Clogb(B,A", "B^C"},
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

pcas_id_t id_trig_identities[ID_NUM_TRIG_IDENTITIES] = {
    {"sin(pi/2_X+C", "cos(X+C"},
    {"cos(pi/2_X+C", "sin(X+C"},

    {"sin(C+2piN", "sin(C"},
    {"sin(C+2pi", "sin(C"},
    {"cos(C+2piN", "cos(C"},
    {"cos(C+2pi", "cos(C"},
    {"tan(C+piN", "tan(C"},
    {"tan(C+pi", "tan(C"},

    {"sin(-C", "-sin(C"},
    {"cos(-C", "cos(C"},
    {"tan(-C", "-tan(C"},

    /*Double angle identities*/
    {"2Csin(X)cos(X", "Csin(2X"},
    {"cos(X)^2_sin(X)^2+C", "cos(2X)+C"},
    {"2cos(X)^2_1+C", "cos(2X)+C"},
    {"1_2sin(X)^2+C", "cos(2X)+C"},

    {"sin(X)^2+cos(X)^2+C", "1+C"},

    /*tan identities*/
    {"Asin(X)/(Bcos(X", "Atan(X)/B"},
    {"Acos(X)/(Bsin(X", "A/(Btan(X"},
    {"Atan(X)cos(X", "Asin(X"},
    {"Asin(X)/(Btan(X", "Acos(X)/B"},
};

pcas_id_t id_trig_constants[ID_NUM_TRIG_CONSTANTS] = {
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
    /*{"tan(pi/2", "inf"},*/
    {"tan(2pi/3", "-sqrt(3"},
    {"tan(3pi/4", "-1"},
    {"tan(5pi/6", "-sqrt(3)/3"},
    {"tan(pi", "0"}
};

pcas_id_t id_hyperbolic[ID_NUM_HYPERBOLIC] = {
    {"cosh(X)_sinh(X)", "e^(-X"},
    {"sinh(X)/cosh(X", "tanh(X"},
    {"cosh(X)^2_sinh(X)^2+C", "1+C"},
};

pcas_id_t id_complex[ID_NUM_COMPLEX] = {
    {"1/i", "-i"},
    {"e^(X(I+Ji", "cos(X)+isin(X"},
    {"(I+Ji)^X", "e^(Xln(I+Ji"},
    {"X^(I+Ji", "e^((I+Ji)ln(X"},
    {"abs(I+Ji", "sqrt(I^2+J^2"},
    {"atan(X/0", "pi/2"}, /*Lol is this cheating?*/
    {"ln(I+Ji", "ln(abs(I+Ji))+iatan(J/I)"},
    {"logb(X,I+Ji", "ln(X)/ln(I+Ji"},
    {"sin(I+Ji", "sin(I)cosh(J)+icos(I)sinh(J"},
    {"cos(I+Ji", "cos(I)cosh(J)_isin(I)sinh(J)"},
    {"tan(I+Ji", "sin(I+Ji)/cos(I+Ji"}
};

typedef pcas_ast_t** Dictionary;

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
    memcpy(dest, source, AMOUNT_SYMBOLS * sizeof(pcas_ast_t*));
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
bool divide_numerical_constants(pcas_ast_t *id, pcas_ast_t *e) {
    pcas_ast_t *child;

    if(!isoptype(id, OP_MULT))
        return false;

    if(is_ast_int(e, 0))
        return false;

    for(child = ast_ChildGet(id, 0); child != NULL; child = child->next) {
        if(child->type == NODE_NUMBER) {
            replace_node(e, ast_MakeBinary(OP_DIV, ast_Copy(e), ast_Copy(child)));
            replace_node(id, ast_MakeBinary(OP_DIV, ast_Copy(id), ast_Copy(child)));

            simplify(e, SIMP_COMMUTATIVE | SIMP_EVAL);
            simplify(id, SIMP_COMMUTATIVE | SIMP_EVAL);

            return true;
        }
    }

    return false;
}

/*Fills in id->to with discovered values from dictionary */
void fill(pcas_ast_t *to, Dictionary dict) {

    if(to->type == NODE_SYMBOL) {
        if(dict_Get(dict, to) != NULL)
            replace_node(to, ast_Copy(dict_Get(dict, to)));
    } else if(to->type == NODE_OPERATOR) {
        pcas_ast_t *child;
        for(child = ast_ChildGet(to, 0); child != NULL; child = child->next) {
            fill(child, dict);
        }
    }

}

bool matches(pcas_ast_t *id, pcas_ast_t *e, Dictionary dict) {

    if(id->type == NODE_SYMBOL && id->op.symbol < SYM_IMAG) {
        if(id->op.symbol == 'N') {
            /*Only integers allowed*/
            if(!(e->type == NODE_NUMBER && mp_rat_is_integer(e->op.num)))
                return false;
        } else if(id->op.symbol == 'I' || id->op.symbol == 'J') {
            /*We assume something is real if it does not have an imaginary node. This could be wrong.*/
            if(has_imaginary_node(e))
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
        pcas_ast_t *dict_copy[AMOUNT_SYMBOLS];

        dict_Copy(dict_copy, dict);

        if(is_op_commutative(optype(id))) {
            /*Order does not matter*/
            bool matched, combined = false, *matched_e_children, *matched_id_children;
            char combined_character = '\0';

            pcas_ast_t *id_copy, *e_copy;

            id_copy = ast_Copy(id);
            e_copy = ast_Copy(e);

            /*Divide numerical constants from each side.*/
            while(divide_numerical_constants(id_copy, e_copy));

            /*Id had numerical coefficients that e did not have.*/
            if(isoptype(e_copy, OP_DIV)) {
                dict_Cleanup(dict_copy);
                ast_Cleanup(id_copy);
                ast_Cleanup(e_copy);
                return false;
            }

            /*Make e_copy an addition or multiplication node with one child so the algorithm below works.*/
            if(!isoptype(e_copy, optype(id)))
                replace_node(e_copy, ast_MakeUnary(id->op.operator.type, ast_Copy(e_copy)));

            if(!isoptype(id_copy, optype(id)))
                replace_node(id_copy, ast_MakeUnary(id->op.operator.type, ast_Copy(id_copy)));

            /*Remove the symbol. Do not simplify commutative. id_copy may be a node with one child.*/
            for(i = 0; i < ast_ChildLength(id_copy); i++) {
                pcas_ast_t *child = ast_ChildGet(id_copy, i);

                if(child->type == NODE_SYMBOL && child->op.symbol < SYM_IMAG && child->op.symbol != 'N') {
                    combined = true;
                    combined_character = child->op.symbol;

                    ast_Cleanup(ast_ChildRemoveIndex(id_copy, i));

                    if(isoptype(id, OP_ADD))
                        dict_copy[combined_character - 'A'] = ast_MakeNumber(num_FromInt(0));
                    else /*OP_MULT*/
                        dict_copy[combined_character - 'A'] = ast_MakeNumber(num_FromInt(1));

                    break;
                }
            }

            /*At this point, id_copy and e_copy are both the same either addition
            or multiplication nodes and we can continue normally*/

            /*We know they will not match if id has more elements than e*/
            if(ast_ChildLength(id_copy) > ast_ChildLength(e_copy)) {
                dict_Cleanup(dict_copy);
                ast_Cleanup(id_copy);
                ast_Cleanup(e_copy);
                return false;
            }

            /*We know we are comparing constants, they need to be same size. */
            if(!combined && ast_ChildLength(id_copy) != ast_ChildLength(e_copy)) {
                dict_Cleanup(dict_copy);
                ast_Cleanup(id_copy);
                ast_Cleanup(e_copy);
                return false;
            }

            /*Loop through all children and match them and record which children got matched.*/

            matched_e_children = calloc(ast_ChildLength(e_copy), sizeof(bool));
            matched_id_children = calloc(ast_ChildLength(id_copy), sizeof(bool));

            do {
                matched = false;

                for(i = 0; i < ast_ChildLength(e_copy); i++) {
                    pcas_ast_t *e_child = ast_ChildGet(e_copy, i);

                    if(matched_e_children[i])
                        continue;

                    for(j = 0; j < ast_ChildLength(id_copy); j++) {
                        pcas_ast_t *dict_copy_copy[AMOUNT_SYMBOLS];
                        pcas_ast_t *id_child = ast_ChildGet(id_copy, j);

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

            /*Make the grouped variable set equal to the nodes not included matched_e_children*/
            if(matched && combined) {
                pcas_ast_t *c;

                c = ast_MakeOperator(id->op.operator.type);

                for(i = 0; i < ast_ChildLength(e_copy); i++) {
                    pcas_ast_t *child = ast_ChildGet(e_copy, i);

                    if(!matched_e_children[i])
                        ast_ChildAppend(c, ast_Copy(child));
                }

                if(ast_ChildLength(c) > 0) {
                    /*If child length is 1, fix it*/
                    simplify(c, SIMP_COMMUTATIVE);

                    /*If the combined parts need to be real and they are or if they don't need to be real*/
                    /*We assume something is real if it does not have an imaginary node. This could be wrong.*/
                    if(!((combined_character == 'I' || combined_character == 'J') && has_imaginary_node(c))) {
                        /*Cleanup and overwrite dummy placeholder*/
                        ast_Cleanup(dict_copy[combined_character - 'A']);
                        dict_copy[combined_character - 'A'] = c;
                    } else {
                        matched = false;
                        ast_Cleanup(c);
                    }
                    
                } else {
                    ast_Cleanup(c);
                }
            }

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
            /*Order and length do matter*/

            if(e->type != NODE_OPERATOR) {
                dict_Cleanup(dict_copy);
                return false;
            }

            if(optype(e) != optype(id) || ast_ChildLength(e) != ast_ChildLength(id)) {
                dict_Cleanup(dict_copy);				
                return false;
            }

            /*Reverse loop to better guess variables for derivative nodes*/
            for(i = ast_ChildLength(e) - 1; i >= 0; i--) {
                pcas_ast_t *e_child = ast_ChildGet(e, i);
                pcas_ast_t *id_child = ast_ChildGet(id, i);
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

/*
    Requires that constants are already evaluated.
*/
bool id_Execute(pcas_ast_t *e, pcas_id_t *id, bool recursive) {
    pcas_ast_t *child;
    pcas_ast_t *dict[AMOUNT_SYMBOLS] = {0};
    bool changed = false;

    if(id->from == NULL || id->to == NULL) {
        if(!id_Load(id)) {
            LOG(("Could not parse identity. from=%s to=%s", id->from_text, id->to_text));
            return false;
        }
    }

    if(matches(id->from, e, dict)) {
        pcas_ast_t *to = ast_Copy(id->to);
        
        fill(to, dict);

        replace_node(e, to);

        /*LOG(("Matched identity from=%s to=%s", id->from_text, id->to_text));*/
        
        changed = true;
    }

    if(recursive && e->type == NODE_OPERATOR) {
        for(child = ast_ChildGet(e, 0); child != NULL; child = child->next)
            changed |= id_Execute(child, id, recursive);
    }

    dict_Cleanup(dict);

    return changed;
}

bool id_Load(pcas_id_t *id) {
    pcas_error_t err;

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

void id_Unload(pcas_id_t *id) {
    if(id->from != NULL)
        ast_Cleanup(id->from);
    id->from = NULL;
    
    if(id->to != NULL)
        ast_Cleanup(id->to);
    id->to = NULL;
}

bool id_ExecuteTable(pcas_ast_t *e, pcas_id_t *table, unsigned table_len, bool recursive) {
    unsigned i;
    bool changed = false;

    for(i = 0; i < table_len; i++) {
        changed |= id_Execute(e, &table[i], recursive);

        /*Break if changed to save time on the calculator becaus chances are good
        that after an identity is applied, we do not need to go through the rest.*/
        simplify(e, SIMP_COMMUTATIVE | SIMP_EVAL);
        if(changed)
            break;
    }

    return changed;
}

void id_UnloadTable(pcas_id_t *table, unsigned table_len) {
    unsigned i;
    for(i = 0; i < table_len; i++)
        id_Unload(&table[i]);
}

#include "derivative.h"
void id_UnloadAll() {
    id_UnloadTable(id_general, ID_NUM_GENERAL);
    id_UnloadTable(id_trig_identities, ID_NUM_TRIG_IDENTITIES);
    id_UnloadTable(id_trig_constants, ID_NUM_TRIG_CONSTANTS);
    id_UnloadTable(id_hyperbolic, ID_NUM_HYPERBOLIC);
    id_UnloadTable(id_complex, ID_NUM_COMPLEX);

    id_UnloadTable(id_derivative, ID_NUM_DERIV);

    id_Unload(&id_deriv_power_rule);
    id_Unload(&id_deriv_constant_rule);
    id_Unload(&id_deriv_product_rule);
}