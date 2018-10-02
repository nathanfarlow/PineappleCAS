/*
    PineappleCAS: the multi-purpose CAS specifically for the TI-84+ CE

    Authors:
    Nathan Farlow
*/

#ifdef COMPILE_PC

#include <stdlib.h>
#include <stdio.h>

#if  defined(_WIN32) && defined(DEBUG)
#include <vld.h>
#endif

#include "tests.h"
#include "../cas/mapping.h"

int main(int argc, char **argv) {

    TestModule *modules;
    unsigned size;
    TestResult result;
    unsigned i;

    result = test_ReadFile("tests/testfile", &modules, &size);

    if(result != TEST_SUCCESS) {
        printf("Error reading test file\n");
        return -1;
    }

    /*Todo: need to remove this system*/
    mapping_Init();

    for(i = 0; i < size; i++) {

        result = test_Run(&modules[i]);
        if(result == TEST_SUCCESS) {
            printf("%s succeeded.\n", modules[i].name);
        }
        else {
            printf("%s failed.\n", modules[i].name);
        }

    }

    mapping_Cleanup();

    free(modules);

    return 0;
}

#else
typedef int make_iso_compilers_happy;
#endif