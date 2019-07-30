#ifndef DERIVATIVE_H_
#define DERIVATIVE_H_

#include "identities.h"

#define ID_NUM_DERIV 18
extern pcas_id_t id_derivative[ID_NUM_DERIV];

extern pcas_id_t id_deriv_power_rule;
extern pcas_id_t id_deriv_constant_rule;
extern pcas_id_t id_deriv_product_rule;

/*Takes the derivative of all derivative nodes*/
bool eval_derivative_nodes(pcas_ast_t *e);
/*Replaces the node with a deriv() node and calls eval_derivative nodes*/
void derivative(pcas_ast_t *e, pcas_ast_t *respect_to, pcas_ast_t *eval_at);

#endif