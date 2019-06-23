/*
    PineappleCAS: the multi-purpose CAS specifically for the TI-84+ CE

    Authors:
    Nathan Farlow
*/

#ifdef COMPILE_PC

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include "../ast.h"
#include "../parser.h"

#include "../dbg.h"

#include "../cas/cas.h"

/*Trim null terminated string*/
uint8_t *trim(char *input, unsigned *len) {
    unsigned i, trimmed_len = 0, trim_index = 0;
    uint8_t *trimmed;

    for(i = 0; i < strlen(input) + 1; i++) {
        if(input[i] != ' ' && input[i] != '\t')
            trimmed_len++;
    }

    trimmed = malloc(trimmed_len * sizeof(uint8_t));

    for(i = 0; i < strlen(input) + 1; i++) {
        if(input[i] != ' ' && input[i] != '\t')
            trimmed[trim_index++] = (uint8_t)input[i];
    }

    *len = trimmed_len - 1; /*Don't include null byte*/

    return trimmed;
}

int test_gcd(int argc, char **argv) {

    uint8_t *trimmed_a, *trimmed_b;
    unsigned trimmed_a_len, trimmed_b_len;

    error_t err;
    ast_t *a, *b, *g;

    uint8_t *output;
    unsigned output_len;

    if(argc <= 2) {
        printf("Usage: ./pineapple \"A^2\" \"ABC\"\n");
        return -1;
    }

    trimmed_a = trim(argv[1], &trimmed_a_len);

    printf("Parsing \"%s\"\n", trimmed_a);

    a = parse(trimmed_a, trimmed_a_len, STR_TABLE, &err);

    printf("%s\n", error_text[err]);

    if(err != E_SUCCESS || a == NULL)
        return -1;

    simplify(a);

    trimmed_b = trim(argv[2], &trimmed_b_len);

    printf("Parsing \"%s\"\n", trimmed_b);

    b = parse(trimmed_b, trimmed_b_len, STR_TABLE, &err);

    printf("%s\n", error_text[err]);

    if(err != E_SUCCESS || b == NULL)
        return -1;

    simplify(b);

    printf("Computing gcd...\n\n");

    g = gcd(a, b);

    dbg_print_tree(g, 4);

    printf("\n");

    output = export_to_binary(g, &output_len, STR_TABLE, &err);

    if(err == E_SUCCESS && output != NULL) {
        printf("Output: ");
        printf("%.*s\n", output_len, output);

        free(output);
    }

    free(trimmed_a);
    free(trimmed_b);

    ast_Cleanup(a);
    ast_Cleanup(b);
    ast_Cleanup(g);

    return 0;
}

int test_simplify(int argc, char **argv) {

    uint8_t *trimmed;
    unsigned trimmed_len;

    error_t err;
    ast_t *e;

    uint8_t *output;
    unsigned output_len;

    if(argc <= 1) {
        printf("Usage: ./pineapple \"sin(2) + 3\"\n");
        return -1;
    }

    trimmed = trim(argv[1], &trimmed_len);

    printf("Parsing \"%s\"\n", trimmed);

    e = parse(trimmed, trimmed_len, STR_TABLE, &err);

    printf("%s\n", error_text[err]);

    if(err == E_SUCCESS && e != NULL) {

        printf("\n");
        dbg_print_tree(e, 4);

        printf("\n");

        printf("Simplifying...\n\n");

        simplify(e);

        dbg_print_tree(e, 4);

        printf("\n");

        output = export_to_binary(e, &output_len, STR_TABLE, &err);

        if(err == E_SUCCESS && output != NULL) {
            printf("Output: ");
            printf("%.*s\n", output_len, output);


            free(output);
        }

    }

    free(trimmed);
    ast_Cleanup(e);

    return 0;
}

int main(int argc, char **argv) {
    return test_gcd(argc, argv);
    /*return test_simplify(argc, argv);*/
}

#else
typedef int make_iso_compilers_happy;
#endif