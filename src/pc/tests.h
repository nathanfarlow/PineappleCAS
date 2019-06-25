#ifndef TESTS_H_
#define TESTS_H_

#include <stdio.h>
#include <stdbool.h>

#define MAX_PAR 256

typedef enum {
	TEST_SIMPLIFY,
	TEST_GCD,
	TEST_FACTOR,
	TEST_EXPAND,

	TEST_INVALID
} TestType;

typedef struct {

	TestType type;

	char arg1[MAX_PAR], arg2[MAX_PAR], arg3[MAX_PAR];

	unsigned line;

} test_t;

/*Returns one test from a line*/
test_t *test_Parse(char *line);
/*Returns an array of tests from a file*/
test_t **test_Load(char *file, unsigned *len);

bool test_Run(test_t *t);
void test_Cleanup(test_t *t);
void test_CleanupArr(test_t **arr, unsigned len);
void test_Print(test_t *t);

#endif