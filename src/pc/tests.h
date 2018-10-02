#ifdef COMPILE_PC

#ifndef TESTS_H_
#define TESTS_H_

#define BUF_SIZE 512

typedef struct {
    char name[BUF_SIZE], file[BUF_SIZE], input[BUF_SIZE],
        output[BUF_SIZE], precision[BUF_SIZE];
} TestModule;

typedef enum {
    TEST_SUCCESS,

    /*Inability to open and read file, maybe becaus it doesn't exist*/
    TEST_IO_ERROR,
    /*Format of file is wrong or corrupt*/
    TEST_BAD_FORMAT,
    /*Parsing failure*/
    TEST_BAD_PARSING,

    /*Wrong answer*/
    TEST_INCORRECT,
} TestResult;

TestResult test_ReadFile(char *file, TestModule **modules, unsigned *size);
TestResult test_Run(TestModule *t);

#endif

#endif