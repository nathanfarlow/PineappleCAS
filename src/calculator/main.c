#ifndef COMPILE_PC

#include <tice.h>
#include <fileioc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../dbg.h"

#include "../ast.h"
#include "../parser.h"

#include "../cas/cas.h"
#include "../cas/mapping.h"

extern char *ftoa(double f, char *buf, int precision);

void main() {
    ti_var_t y1;

    const uint8_t *data;
    uint16_t size;

    os_ClrHome();
    ti_CloseAll();

    y1 = ti_OpenVar(ti_Y1, "r", TI_EQU_TYPE);

    if(y1) {
        error err;
        ast_t *e;

        data = ti_GetDataPtr(y1);
        size = ti_GetSize(y1);

        e = parse(data, size, &err);

        ti_Close(y1);

        if(err != E_SUCCESS) {
            LOG(("Couldn't parse equation, reason: %s", error_text[err]));
        } else {
            double value;
            char buffer[50];

            ast_t *x = ast_MakeNumber(num_CreateDecimal("2.53"));

            dbg_print_tree(e, 4);

            mapping_Init();
            mapping_Set('X', x);

            value = approximate(e, &err);
            if(err == E_SUCCESS) {
                ftoa(value, buffer, 10);
                DBG(("Eval: %s\n\n", buffer));
            } else {
                DBG(("Unable to evaluate, reason: %s\n\n", error_text[err]));
            }

            simplify(e);

            DBG(("Simplified:\n"));
            dbg_print_tree(e, 4);

            value = approximate(e, &err);
            if(err == E_SUCCESS) {
                ftoa(value, buffer, 10);
                DBG(("Eval: %s\n", buffer));
            } else {
                DBG(("Unable to evaluate, reason: %s\n\n", error_text[err]));
            }

        }

        ast_Cleanup(e);

    } else {
        LOG(("Couldn't open equation"));
    }

    _OS(asm_ClrTxtShd);
}

#else
typedef int make_iso_compilers_happy;
#endif