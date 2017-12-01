#ifndef PARSER_H_
#define PARSER_H_

#include "num.h"

typedef enum {
	TOK_SIN
} TokenType;

typedef struct _Token {
	TokenType type;
	union {
		num_t value;
		char symbol;
	};
} token_t;

#endif