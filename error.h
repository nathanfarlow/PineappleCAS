#ifndef _ERROR_H_
#define _ERROR_H_

typedef enum _Error {
	E_SUCCESS,

    //trying to add nodes to a NODE_NUMBER for instance
    E_AST_NOT_ALLOWED,
    E_AST_OUT_OF_BOUNDS,

	ERROR_AMOUNT
} error;

extern const char *text[ERROR_AMOUNT];

#endif