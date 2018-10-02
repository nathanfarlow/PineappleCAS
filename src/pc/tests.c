#ifdef COMPILE_PC

#include "tests.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "../dbg.h"

#include "yvar.h"

#include "../parser.h"

#include "../cas/cas.h"
#include "../cas/mapping.h"

/*Read string until ':' and discard ':'*/
TestResult read_string(FILE *f, char *buffer) {

    unsigned i;

    if(feof(f))
        return TEST_BAD_FORMAT;

    for(i = 0; i < BUF_SIZE; i++) {
        char a = fgetc(f);
        if(a == ':') {
            buffer[i] = 0;
            return TEST_SUCCESS;
        }
        buffer[i] = a;
    }

    if(i == BUF_SIZE)
        return TEST_BAD_FORMAT;

    return TEST_SUCCESS;
}

#define READ_STRING_TO(buf)  result = read_string(f, buf); \
                            if(result != TEST_SUCCESS) return result;

TestResult test_ReadFile(char *file, TestModule **modules, unsigned *size) {
    FILE *f = NULL;
    TestResult result;

    unsigned index;

#ifdef _WIN32
    fopen_s(&f, file, "rb");
#else
    f = fopen(file, "rb");
#endif

    if(f == NULL)
        return TEST_IO_ERROR;


    /*Count the newlines in the file lol*/
    *size = 1;
    while(!feof(f)) *size += (fgetc(f) == '\n');

    if(*size == 0) {
        fclose(f);
        return TEST_BAD_FORMAT;
    }

    *modules = malloc(sizeof(TestModule) * *size);

    rewind(f);
    
    for(index = 0; index < *size; index++) {
        char c;

        READ_STRING_TO((*modules)[index].name);
        READ_STRING_TO((*modules)[index].file);
        READ_STRING_TO((*modules)[index].input);
        READ_STRING_TO((*modules)[index].output);
        READ_STRING_TO((*modules)[index].precision);
        /*discard new lines*/
        do {
            c = fgetc(f);
        } while(c == '\n' || c == '\r');
        fseek(f, -1, SEEK_CUR);
    }

    fclose(f);
    return TEST_SUCCESS;
}


TestResult test_Run(TestModule *t) {
    FILE *f = NULL;

    yvar_t yvar;
    ast_t *e, *x;

    error_t err;

    double value;
    int precision;

    char buffer[BUF_SIZE + 10] = {0};
#ifdef _WIN32
    strcat_s(buffer, BUF_SIZE + 10, "tests/");
    strncat_s(buffer, BUF_SIZE + 10, t->file, BUF_SIZE);
    fopen_s(&f, buffer, "rb");
#else
    strcat(buffer, "tests/");
    strncat(buffer, t->file, BUF_SIZE);
    f = fopen(buffer, "rb");
#endif

    if(f == NULL)
        return TEST_IO_ERROR;

    if(yvar_Read(&yvar, f) != 0) {
        printf("Corrupt or invalid 8xy file.\n");
        fclose(f);
        return TEST_BAD_FORMAT;
    }

    e = parse(yvar.data, yvar.yvar_data_len, &err);

    if(err != E_SUCCESS) {
        fclose(f);
        yvar_Cleanup(&yvar);
        return TEST_BAD_PARSING;
    }

    fclose(f);

    x = ast_MakeNumber(num_CreateDecimal(t->input));

    /*TODO: need to remove this system later on*/
    mapping_Set('X', x);

    value = approximate(e, &err);

    if(err != E_SUCCESS) {
        ast_Cleanup(e);
        mapping_Cleanup();
        yvar_Cleanup(&yvar);
        return TEST_BAD_PARSING;
    }

    precision = atoi(t->precision);
    if(precision < 0) {
        ast_Cleanup(e);
        mapping_Cleanup();
        yvar_Cleanup(&yvar);
        return TEST_BAD_FORMAT;
    }
    
#ifdef _WIN32
    sprintf_s(buffer, BUF_SIZE, "%f", value);
#else
    sprintf(buffer, "%f", value);
#endif

    ast_Cleanup(e);
    yvar_Cleanup(&yvar);

    if(strncmp(buffer, t->output, (BUF_SIZE < precision ? BUF_SIZE : precision)) != 0)
        return TEST_INCORRECT;

    return TEST_SUCCESS;
}

#else
typedef int make_iso_compilers_happy;
#endif
