#ifndef PINEAPPLE_DEBUG_H_
#define PINEAPPLE_DEBUG_H_

#include <stdio.h>
#include <string.h>

#include "ast.h"

#ifdef DEBUG

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#endif

#ifdef COMPILE_PC
#define DBG(args) (printf args)
#define LOG(args) (printf("LOG: %s:%d ", __FILENAME__, __LINE__), printf args, printf("\n"))
#else

#define _TICE
#include <debug.h>

void ti_debug(const char *format, ...);

#define DBG(args) (ti_debug args)
#define LOG(args) (ti_debug("LOG: %s:%d ", __FILENAME__, __LINE__), ti_debug args, ti_debug("\n"))

#endif

#else

#define DBG(args)
#define LOG(args)

#endif

void dbg_print_tree(pcas_ast_t *e, unsigned indent);
unsigned dbg_count_nodes(pcas_ast_t *e);

#endif