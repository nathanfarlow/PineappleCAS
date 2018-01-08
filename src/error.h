#ifndef ERROR_H_
#define ERROR_H_

typedef enum _Error {
	E_SUCCESS,

    /*trying to add nodes to a NODE_NUMBER for instance*/
    E_AST_NOT_ALLOWED,
    E_AST_OUT_OF_BOUNDS,

    E_TOK_INVALID,

	ERROR_AMOUNT
} error;

extern const char *error_text[ERROR_AMOUNT];

#endif