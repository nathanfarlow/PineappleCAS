#ifndef COMPILE_PC

#include <tice.h>
#include <fileioc.h>

#include "../dbg.h"

#include "../parser.h"
#include "../cas/cas.h"

void main() {
	ti_var_t y1;

	const uint8_t *data;
	uint16_t size;
	
	os_ClrHome();
	ti_CloseAll();

	/*Read equation from Y1*/
	y1 = ti_OpenVar(ti_Y1, "r", TI_EQU_TYPE);

	if(y1 != NULL) {
		error_t err;
		ast_t *e;

		data = ti_GetDataPtr(y1);
		size = ti_GetSize(y1);

		e = parse(data, size, TI_TABLE, &err);

		ti_Close(y1);

		/*Simplify and export to Y2*/
		if(err == E_SUCCESS) {

			unsigned bin_len;
			uint8_t *bin;

			ti_var_t y2;

			simplify(e);

			bin = export_to_binary(e, &bin_len, TI_TABLE, &err);

			y2 = ti_OpenVar(ti_Y2, "w", TI_EQU_TYPE);
			ti_Write(bin, bin_len, 1, y2);
			ti_Close(y2);

		} else {
            LOG(("Couldn't parse equation, reason: %s", error_text[err]));
		}

		ast_Cleanup(e);

	} else {
		LOG(("Couldn't open equation"));
	}

    /*Try to avoid that weird effect on the graph screen*/
	_OS(asm_ClrTxtShd);
}

#else
typedef int make_iso_compilers_happy;
#endif