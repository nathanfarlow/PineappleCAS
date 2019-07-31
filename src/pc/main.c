/*
    PineappleCAS: the multi-purpose CAS specifically for the TI-84+ CE

    Authors:
    Nathan Farlow
*/

#ifdef COMPILE_PC

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <string.h>

#include "../ast.h"
#include "../parser.h"

#include "../dbg.h"

#include "../cas/cas.h"

#include "tests.h"

void display_help() {
    printf("Usage: ./pineapple [operation] [args]\n");
    printf("Valid operations include:\n");
    printf("\ttest [file]\t\t\tRuns all tests in file\n");
    printf("\tsimplify [expression]\t\tSimplifies expression. Also evaluates constants.\n");
    printf("\tgcd [expression1] [expression2]\tPrints the GCD of the two expressions\n");
    printf("\tfactor [expression]\t\tFactors expression\n");
    printf("\texpand [expression]\t\tExpands expression\n");
    printf("\tderivative [expression] [respect to] [(optional) eval at]\n");
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

    pcas_error_t err;
    pcas_ast_t *a, *b, *g;

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
    simplify_canonical_form(g);

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
extern bool simplify_periodic(pcas_ast_t *e);
int run_simplify(int argc, char **argv) {

    uint8_t *trimmed;
    unsigned trimmed_len;

    pcas_error_t err;
    pcas_ast_t *e;

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
        simplify_canonical_form(e);

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

    pcas_error_t err;
    pcas_ast_t *e;

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

        dbg_print_tree(e, 4);

        printf("\n");

        printf("Factoring...\n\n");

        factor(e, FAC_ALL);

        /*simplify(e, SIMP_ALL);*/
        simplify_canonical_form(e);

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

    pcas_error_t err;
    pcas_ast_t *e;

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

        dbg_print_tree(e, 4);

        printf("\n");

        printf("Expanding...\n\n");

        simplify(e, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL);
        expand(e, EXP_ALL);
        simplify(e, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL | SIMP_LIKE_TERMS | SIMP_EVAL);
        simplify_canonical_form(e);

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

    clock_t delta;

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

    delta = clock();
    for(i = 0; i < len; i++) {
        test_t *t = arr[i];
        printf("Running test %d/%d on line %d... ", i + 1, len, t->line);
        if(!test_Run(t)) {
            puts("[FAIL]");
            failed++;
        } else {
            puts("[OK]");
            passed++;
        }
    }
    delta = clock() - delta;

    test_CleanupArr(arr, len);

    printf("Finished in %li microseconds.\n", delta / (CLOCKS_PER_SEC / 1000));
    printf("Passed: %u/%u, Failed: %u/%u\n", passed, len, failed, len);

    if(failed == 0) {
        printf("All tests passed!\n");
        return 0;
    }

    return -1;
}

int run_derivative(int argc, char **argv) {

    uint8_t *trimmed;
    unsigned trimmed_len;

    pcas_error_t err;
    pcas_ast_t *e = NULL, *respect_to = NULL, *at = NULL;

    uint8_t *output;
    unsigned output_len;

    if(argc <= 3) {
        display_help();
        return -1;
    }

    trimmed = trim(argv[2], &trimmed_len);
    printf("Parsing \"%s\"\n", trimmed);
    e = parse(trimmed, trimmed_len, str_table, &err);
    printf("%s\n", error_text[err]);
    free(trimmed);

    if(err == E_SUCCESS) {
        trimmed = trim(argv[3], &trimmed_len);
        printf("Parsing \"%s\"\n", trimmed);
        respect_to = parse(trimmed, trimmed_len, str_table, &err);
        printf("%s\n", error_text[err]);
        free(trimmed);
    }

    if(err == E_SUCCESS) {

        if(argc >= 5) {
            trimmed = trim(argv[4], &trimmed_len);
            printf("Parsing \"%s\"\n", trimmed);
            at = parse(trimmed, trimmed_len, str_table, &err);
            printf("%s\n", error_text[err]);
            free(trimmed);
        } else {
            at = ast_Copy(respect_to);
        }

    }

    if(err == E_SUCCESS && e != NULL && respect_to != NULL && at != NULL) {

        printf("\n");
        dbg_print_tree(e, 4);

        printf("\n");

        printf("Simplifying...\n\n");

        simplify(e, SIMP_ALL);

        dbg_print_tree(e, 4);

        printf("\n");

        printf("Taking derivative...\n");

        derivative(e, respect_to, at);

        printf("Simplifying...\n\n");

        simplify(e, SIMP_ALL);
        simplify_canonical_form(e);

        dbg_print_tree(e, 4);
        printf("\n");

        output = export_to_binary(e, &output_len, str_table, &err);

        if(err == E_SUCCESS && output != NULL) {
            printf("Output: ");
            printf("%.*s\n", output_len, output);

            free(output);
        }

    }

    ast_Cleanup(e);
    ast_Cleanup(respect_to);
    ast_Cleanup(at);

    return 0;
}

int main(int argc, char **argv) {
    int ret;

    if(argc > 1) {
        if(!strcmp(argv[1], "test"))            ret = run_test(argc, argv);
        else if(!strcmp(argv[1], "simplify"))   ret = run_simplify(argc, argv);
        else if(!strcmp(argv[1], "gcd"))        ret = run_gcd(argc, argv);
        else if(!strcmp(argv[1], "factor"))     ret = run_factor(argc, argv);
        else if(!strcmp(argv[1], "expand"))     ret = run_expand(argc, argv);
        else if(!strcmp(argv[1], "derivative")) ret = run_derivative(argc, argv);
        else {
            display_help();
            return -1;
        }
        id_UnloadAll();
        return ret;
    }

    display_help();
    return -1;
}

#else
typedef int make_iso_compilers_happy;
#endif