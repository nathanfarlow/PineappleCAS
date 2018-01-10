#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdio.h>

#include "ast.h"

#ifdef DEBUG

#ifdef COMPILE_PC
#define DBG(args) (printf args)
#define LOG(args) (printf("LOG: %s:%d ", __FILE__, __LINE__), printf args, printf("\n"))
#endif

#else

#define DBG(args)
#define LOG(args)

#endif

void dbg_print_tree(ast_t *e, unsigned indent);
unsigned dbg_count_nodes(ast_t *e);

#endif