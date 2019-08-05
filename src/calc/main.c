#ifndef COMPILE_PC

#include "gui.h"
#include "interface.h"

void main() {

	if(interface_Valid())
		interface_Run();
	else
    	gui_Run();
}

#else
typedef int make_iso_compilers_happy;
#endif