#ifndef COMPILE_PC

#include "gui.h"
#include "interface.h"

int main() {

    if(interface_Valid())
        interface_Run();
    else
        gui_Run();

    return 0;
}

#else
typedef int make_iso_compilers_happy;
#endif