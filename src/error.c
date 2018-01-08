#include "error.h"

const char *error_text[ERROR_AMOUNT] = {
	"Success",

    "AST operation not allowed",
    "AST index out of bounds",

    "Invalid token",

    "Bad operator",
    "Unmatched closing parentheses",
    "Bad comma"
};