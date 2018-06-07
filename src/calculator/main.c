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
#include "../export.h"

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
        error_t err;
        ast_t *e;

        data = ti_GetDataPtr(y1);
        size = ti_GetSize(y1);

        e = parse(data, size, &err);

        ti_Close(y1);

        if(err != E_SUCCESS) {
            LOG(("Couldn't parse equation, reason: %s", error_text[err]));
        } else {
            unsigned binary_len;
            uint8_t *binary;

            ti_var_t y2;

            simplify(e);

            binary = export_to_binary(e, &binary_len, &err);

            y2 = ti_OpenVar(ti_Y2, "w", TI_EQU_TYPE);
            ti_Write(binary, binary_len, 1, y2);
            ti_Close(y2);
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