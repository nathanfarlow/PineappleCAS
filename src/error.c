#include "error.h"

const char *error_text[AMOUNT_ERRORS] = {
    "Success",

    "Generic failure - Please report!",

    "AST operation not allowed",
    "AST index out of bounds",

    "Invalid token",

    "Bad operator",
    "Unmatched closing parentheses",
    "Bad comma",

    "No mapping for symbol",
};