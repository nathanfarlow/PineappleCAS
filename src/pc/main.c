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
#include "../cas/identities.h"

#include "tests.h"

void display_help() {
    printf("Usage: ./pineapple [operation] [args]\n");
    printf("Valid operations include:\n");
    printf("\ttest [file]\t\t\tRuns all tests in file\n");
    printf("\tsimplify [expression]\t\tSimplifies expression. Also evaluates constants.\n");
    printf("\tgcd [expression1] [expression2]\tPrints the GCD of the two expressions\n");
    printf("\tfactor [expression]\t\tFactors expression\n");
    printf("\texpand [expression]\t\tExpands expression\n");
}

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

int run_gcd(int argc, char **argv) {

    uint8_t *trimmed_a, *trimmed_b;
    unsigned trimmed_a_len, trimmed_b_len;

    error_t err;
    ast_t *a, *b, *g;

    uint8_t *output;
    unsigned output_len;

    if(argc <= 3) {
        display_help();
        return -1;
    }

    trimmed_a = trim(argv[2], &trimmed_a_len);

    printf("Parsing \"%s\"\n", trimmed_a);

    a = parse(trimmed_a, trimmed_a_len, str_table, &err);

    printf("%s\n", error_text[err]);

    if(err != E_SUCCESS || a == NULL)
        return -1;

    simplify(a, SIMP_ALL);

    trimmed_b = trim(argv[3], &trimmed_b_len);

    printf("Parsing \"%s\"\n", trimmed_b);

    b = parse(trimmed_b, trimmed_b_len, str_table, &err);

    printf("%s\n", error_text[err]);

    if(err != E_SUCCESS || b == NULL)
        return -1;

    simplify(b, SIMP_ALL);

    printf("Computing gcd...\n\n");

    g = gcd(a, b);

    simplify(g, SIMP_ALL);
    simplify(g, SIMP_CANONICAL_FORM);

    dbg_print_tree(g, 4);

    printf("\n");

    output = export_to_binary(g, &output_len, str_table, &err);

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

int run_simplify(int argc, char **argv) {

    uint8_t *trimmed;
    unsigned trimmed_len;

    error_t err;
    ast_t *e;

    uint8_t *output;
    unsigned output_len;

    if(argc <= 2) {
        display_help();
        return -1;
    }

    trimmed = trim(argv[2], &trimmed_len);

    printf("Parsing \"%s\"\n", trimmed);

    e = parse(trimmed, trimmed_len, str_table, &err);

    printf("%s\n", error_text[err]);

    if(err == E_SUCCESS && e != NULL) {

        printf("\n");
        dbg_print_tree(e, 4);

        printf("\n");

        printf("Simplifying...\n\n");

        simplify(e, SIMP_ALL);
        simplify(e, SIMP_CANONICAL_FORM);

        dbg_print_tree(e, 4);

        printf("\n");

        output = export_to_binary(e, &output_len, str_table, &err);

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

int run_factor(int argc, char **argv) {
uint8_t *trimmed;
    unsigned trimmed_len;

    error_t err;
    ast_t *e;

    uint8_t *output;
    unsigned output_len;

    if(argc <= 2) {
        display_help();
        return -1;
    }

    trimmed = trim(argv[2], &trimmed_len);

    printf("Parsing \"%s\"\n", trimmed);

    e = parse(trimmed, trimmed_len, str_table, &err);

    printf("%s\n", error_text[err]);

    if(err == E_SUCCESS && e != NULL) {

        printf("\n");
        dbg_print_tree(e, 4);

        printf("\n");

        printf("Simplifying...\n\n");

        simplify(e, SIMP_ALL);
        simplify(e, SIMP_CANONICAL_FORM);

        dbg_print_tree(e, 4);

        printf("\n");

        printf("Factoring...\n\n");

        factor(e, FAC_ALL);

        simplify(e, SIMP_ALL);
        simplify(e, SIMP_CANONICAL_FORM);

        dbg_print_tree(e, 4);
        printf("\n");

        output = export_to_binary(e, &output_len, str_table, &err);

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

int run_expand(int argc, char **argv) {

    uint8_t *trimmed;
    unsigned trimmed_len;

    error_t err;
    ast_t *e;

    uint8_t *output;
    unsigned output_len;

    if(argc <= 2) {
        display_help();
        return -1;
    }

    trimmed = trim(argv[2], &trimmed_len);

    printf("Parsing \"%s\"\n", trimmed);

    e = parse(trimmed, trimmed_len, str_table, &err);

    printf("%s\n", error_text[err]);

    if(err == E_SUCCESS && e != NULL) {

        printf("\n");
        dbg_print_tree(e, 4);

        printf("\n");

        printf("Simplifying...\n\n");

        simplify(e, SIMP_ALL);
        simplify(e, SIMP_CANONICAL_FORM);

        dbg_print_tree(e, 4);

        printf("\n");

        printf("Expanding...\n\n");

        expand(e, EXP_ALL);

        dbg_print_tree(e, 4);
        printf("\n");

        output = export_to_binary(e, &output_len, str_table, &err);

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

int run_test(int argc, char **argv) {
    unsigned len, i, failed = 0, passed = 0;
    test_t **arr;

    if(argc < 3) {
        display_help();
        return -1;
    }

    arr = test_Load(argv[2], &len);

    if(arr == NULL) {
        printf("Could not load test file.\n");
        return -1;
    }

    printf("Running tests...\n");
    
    for(i = 0; i < len; i++) {
        test_t *t = arr[i];
        if(!test_Run(t))
            failed++;
        else
            passed++;
    }
    
    test_CleanupArr(arr, len);

    printf("Passed: %u/%u, Failed: %u/%u\n", passed, len, failed, len);

    if(failed == 0) {
        printf("All tests passed!\n");
        return 0;
    }

    return -1;
}

int main(int argc, char **argv) {
    int ret;

    if(argc > 1) {
        if(!strcmp(argv[1], "test"))            ret = run_test(argc, argv);
        else if(!strcmp(argv[1], "simplify"))   ret = run_simplify(argc, argv);
        else if(!strcmp(argv[1], "gcd"))        ret = run_gcd(argc, argv);
        else if(!strcmp(argv[1], "factor"))     ret = run_factor(argc, argv);
        else if(!strcmp(argv[1], "expand"))     ret = run_expand(argc, argv);

        id_UnloadAll();
        return ret;
    }

    display_help();
    return -1;
}

#else
typedef int make_iso_compilers_happy;
#endif