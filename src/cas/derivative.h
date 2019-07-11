#ifndef DERIVATIVE_H_
#define DERIVATIVE_H_

#include "identities.h"

#define ID_NUM_DERIV 18
extern id_t id_derivative[ID_NUM_DERIV];

extern id_t id_deriv_power_rule;
extern id_t id_deriv_constant_rule;
extern id_t id_deriv_product_rule;

/*Takes the derivative of all derivative nodes*/
bool eval_derivative_nodes(ast_t *e);

#endif