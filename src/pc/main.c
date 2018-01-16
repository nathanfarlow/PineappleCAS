/*
    PineappleCAS: the multi-purpose CAS specifically for the TI-84+ CE

    Authors:
    Nathan Farlow
*/

#ifdef COMPILE_PC

#include <stdlib.h>

#include "../debug.h"
#include "../parser.h"

#include "../cas/cas.h"
#include "../cas/mapping.h"

#include "yvar.h"

int main(int argc, char **argv) {

    error err;

    FILE *file;
    yvar_t yvar;

    ast_t *e;
    ast_t *x;

    double value;

    if (argc <= 1) {
        printf("Usage: pineapple.exe /path/to/your/yvar.8xy\n");
        return 1;
    }

#ifdef _WIN32
	fopen_s(&file, argv[1], "rb");
#else
    file = fopen(argv[1], "rb");
#endif

    if (!file) {
        printf("File not found.\n");
        return 1;
    }

    if (yvar_Read(&yvar, file) != 0) {
        printf("Corrupt or invalid 8xy file.\n");
        fclose(file);
        return 1;
    }

    e = parse(yvar.data, yvar.yvar_data_len, &err);

    if (err != E_SUCCESS) {
        printf("Unable to parse ast, reason: %s\n", error_text[err]);
        fclose(file);
        yvar_Cleanup(&yvar);
        return 1;
    }

    fclose(file);
    
    DBG(("Node count: %i\n", dbg_count_nodes(e)));
    dbg_print_tree(e, 4);

    x = ast_MakeNumber(num_CreateDecimal("2.53"));

    mapping_Init();
    mapping_Set('X', x);

    value = approximate(e, &err);
    if(err == E_SUCCESS)
        DBG(("Eval: %f\n\n", value));
    else
        DBG(("Unable to evaluate, reason: %s\n", error_text[err]));

    DBG(("Simplified:\n"));
    simplify(e);
    dbg_print_tree(e, 4);

    value = approximate(e, &err);
    if(err == E_SUCCESS)
        DBG(("Eval: %f\n\n", value));
    else
        DBG(("Unable to evaluate, reason: %s\n", error_text[err]));

    ast_Cleanup(e);

    mapping_Cleanup();

    yvar_Cleanup(&yvar);

    return 0;
}

#endif