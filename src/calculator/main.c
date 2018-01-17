#ifndef COMPILE_PC

#include <tice.h>
#include <fileioc.h>

#include <stdlib.h>

#include "../dbg.h"

#include "../ast.h"
#include "../parser.h"

#include "../cas/cas.h"
#include "../cas/mapping.h"

void print_text(int8_t xpos, int8_t ypos, const char *text) {
    os_SetCursorPos(ypos, xpos);
    os_PutStrFull(text);
}

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
            double value1, value2;
            ast_t *x = ast_MakeNumber(num_CreateDecimal("2.53"));

            dbg_print_tree(e, 4);

            mapping_Init();
            mapping_Set('X', x);

            value1 = approximate(e, &err);

            simplify(e);

            DBG(("Simplified:\n"));
            dbg_print_tree(e, 4);

            value2 = approximate(e, &err);
            if(err == E_SUCCESS) {
                if(value1 == value2) {
                    DBG(("approximate() returned the same result."));
                } else {
                    DBG(("approximate() DID NOT return the same result."));
                }
            } else
                DBG(("Unable to evaluate, reason: %s\n\n", error_text[err]));
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