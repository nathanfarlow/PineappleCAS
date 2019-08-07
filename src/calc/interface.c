#ifndef COMPILE_PC

#include "interface.h"

#include <stdlib.h>
#include <string.h>

#include "../ast.h"
#include "../parser.h"
#include "../cas/cas.h"

#define TI_SPACE 0x29
#define TI_COMMA 0x2B

/*Returns a new string with TI spaces removed*/
uint8_t *trim(uint8_t *input, unsigned input_len, unsigned *trimmed_len) {
    unsigned i, trim_index = 0;
    uint8_t *trimmed;

    *trimmed_len = 0;

    for(i = 0; i < input_len; i++) {
        if(input[i] != TI_SPACE)
            (*trimmed_len)++;
    }

    trimmed = malloc((*trimmed_len) * sizeof(char));

    for(i = 0; i < input_len; i++) {
        if(input[i] != TI_SPACE)
            trimmed[trim_index++] = input[i];
    }

    return trimmed;
}

bool is_var_string_type(ti_var_t var) {
    /*Check if 4 low bits from first byte of vat data is 0x4*/
    return (*((uint8_t*)ti_GetVATPtr(var)) & 0xFu) == 0x4u;
}

/*Read trimmed string from Ans variable*/
uint8_t *read_ans(unsigned *ans_len) {
    ti_var_t v;
    const uint8_t *data;
    uint8_t *str, *trimmed = NULL;

    uint16_t size;

    ti_CloseAll();

    v = ti_OpenVar(ti_Ans, "r", TI_STRING_TYPE);

    /*If Ans is a string*/
    if(is_var_string_type(v)) {
        data = ti_GetDataPtr(v);
        size = ti_GetSize(v);

        str = malloc(sizeof(char) * size);
        memcpy(str, data, size);

        trimmed = trim(str, (unsigned)size, ans_len);
        free(str);
    } else {
        *ans_len = 0;
    }

    ti_Close(v);

    return trimmed;
}

typedef struct {
    unsigned amount;
    uint8_t **args;
    /*Array for the length of each argv. Have to include because argument may have null chars so we can't use strlen()*/
    unsigned *arg_len;
} arg_list;

uint8_t *parse_one(uint8_t *input, unsigned input_len, unsigned start, unsigned *arg_len) {
    unsigned i;
    uint8_t *ret = NULL;

    *arg_len = 0;

    for(i = start; i < input_len; i++) {
        if(input[i] == TI_COMMA)
            break;
        (*arg_len)++;
    }

    if(*arg_len > 0)
        ret = malloc(sizeof(char) * (*arg_len));

    for(i = 0; i < *arg_len; i++) {
        ret[i] = input[start + i];
    }

    return ret;
}

bool parse_args(uint8_t *input, unsigned input_len, arg_list *args) {
    unsigned i, arg_index = 0;

    if(input_len == 0)
        return false;

    /*Calculate amount of arguments*/
    args->amount = 1;
    for(i = 0; i < input_len; i++) {
        if(input[i] == TI_COMMA)
            args->amount++;
    }

    args->args = malloc(sizeof(char*) * args->amount);
    args->arg_len = malloc(sizeof(unsigned) * args->amount);

    /*Parse arguments one by one*/
    for(i = 0; i < args->amount; i++) {
        args->args[i] = parse_one(input, input_len, arg_index, &args->arg_len[i]);
        arg_index += args->arg_len[i] + 1; /*+ 1 to eliminate the comma*/
    }

    return true;
}

void cleanup_args(arg_list *args) {
    unsigned i;

    for(i = 0; i < args->amount; i++) {
        free(args->args[i]);
    }

    free(args->args);
    free(args->arg_len);
}

void write_ans(int val) {
    real_t real;
    real = os_Int24ToReal(val);
    ti_SetVar(TI_REAL_TYPE, ti_Ans, &real);
}

void success() {
    write_ans(1);
}

void fail(char *message) {
    char buffer[150] = {0};

    if(message != NULL) {
        os_ClrHome();

        os_SetCursorPos(0, 0);

        if(strlen(message) < 50) {
            sprintf(buffer, "Wrong command syntax. %s. See github.com/nathanfarlow/PineappleCAS for usage.", message);
            os_PutStrFull(buffer);   
        }

        while (!os_GetCSC());
    }
    write_ans(0);
}

#define interface_assert(condition, message) do {if(!(condition)) {fail(message); return;}} while(0)

/*Valid symbols are Y1 through Y0, Str1 through Str0, and Ans*/
bool tok_valid(uint8_t *symbol, unsigned symbol_len) {
    if(symbol_len != 4)
        /*Check if symbol is Ans*/
        return symbol_len == 3 && symbol[0] == 0x72u && symbol[1] == 0u && symbol[2] == 0u;

    /*Check if symbol is Y1 through Y0*/
    if(symbol[0] == 0x5Eu) {
        return symbol[1] >= 0x10u && symbol[1] <= 0x19u && symbol[2] == 0u && symbol[3] == 0u;
    }
    /*Check if symbol is Str1 through Str0*/
    else if(symbol[0] == 0xAAu) {
        return symbol[1] >= 0x0u && symbol[1] <= 0x9u && symbol[2] == 0u && symbol[3] == 0u;
    }

    return false;
}
/*Add two null terminators to symbol*/
void tok_fix(uint8_t **symbol, unsigned *symbol_len) {
    if(*symbol_len == 0)
        return;

    *symbol = realloc(*symbol, sizeof(char) * (*symbol_len + 2));
    (*symbol)[*symbol_len] = 0;
    (*symbol)[*symbol_len + 1] = 0;
    (*symbol_len) += 2;
}

pcas_ast_t *parse_from_tok(uint8_t *tok, pcas_error_t *err) {
    ti_var_t var;

    ti_CloseAll();

    var = ti_OpenVar((char*)tok, "r", tok[0] == 0x5Eu ? TI_EQU_TYPE : TI_STRING_TYPE);

    /*If we're opening Ans or a string and the type is not a string type*/
    if(tok[0] != 0x5Eu && !is_var_string_type(var)) {
        *err = E_GENERIC;
        return NULL;
    }

    if(var != NULL) {
        const uint8_t *data;
        uint16_t size;

        pcas_ast_t *result;

        data = ti_GetDataPtr(var);
        size = ti_GetSize(var);

        result = parse(data, size, ti_table, err);

        ti_Close(var);

        if(*err == E_SUCCESS)
            return result;

        return NULL;
    }

    *err = E_GENERIC;
    return NULL;
}

void write_to_tok(uint8_t *tok, pcas_ast_t *expression, pcas_error_t *err) {
    unsigned bin_len;
    uint8_t *bin;
    ti_var_t var;

    ti_CloseAll();

    var = ti_OpenVar((char*)tok, "w", tok[0] == 0x5Eu ? TI_EQU_TYPE : TI_STRING_TYPE);

    if(var != NULL) {

        /*Write to var*/
        bin = export_to_binary(expression, &bin_len, ti_table, err);
        ti_Write(bin, bin_len, 1, var);

        /*If var is a yvar, enable it*/
        if(var <= 9) {
            /*Thanks Mateo: https://www.cemetech.net/forum/viewtopic.php?t=15947*/
            uint8_t *status;
            status = ti_GetVATPtr(var);
            status--;
            *status |= 1;
        }
        
        ti_Close(var);
    } else {
        *err = E_GENERIC;
        return;
    }

    *err = E_SUCCESS;
}

/*
    Syntax: SIMP,Y1,Y2 or SIMP,Y1,Y2,010101

    None set by default.

    Boolean 1 = Basic identities
    Boolean 2 = Trig identities
    Boolean 3 = Hyperbolic identities
    Boolean 4 = Complex identities
    Boolean 5 = Evaluate trig constants
    Boolean 6 = Evaluate inverse trig constants
*/
void interface_Simplify(arg_list *args) {
    pcas_ast_t *expression;
    pcas_error_t err;

    unsigned short flags = SIMP_ALL;

    uint8_t *input, *output;
    unsigned input_len, output_len;

    interface_assert(args->amount >= 3, "Not enough arguments");

    tok_fix(&args->args[1], &args->arg_len[1]);
    tok_fix(&args->args[2], &args->arg_len[2]);

    input = args->args[1];
    output = args->args[2];

    input_len = args->arg_len[1];
    output_len = args->arg_len[2];

    interface_assert(tok_valid(input, input_len), "Not a valid input variable");
    interface_assert(tok_valid(output, output_len), "Not a valid output variable");

    if(args->amount >= 4) {
        uint8_t *options;
        unsigned option_len;

        unsigned i = 0;

        flags ^= SIMP_ID_ALL;

        options = args->args[3];
        option_len = args->arg_len[3];

        interface_assert(option_len == 6, "Wrong number of boolean options");

        for(i = 0; i < 6; i++)
            interface_assert(options[i] == '0' || options[i] == '1', "Boolean option must be 0 or 1");

        if(options[0] == '1') flags |= SIMP_ID_GENERAL;
        if(options[1] == '1') flags |= SIMP_ID_TRIG;
        if(options[2] == '1') flags |= SIMP_ID_HYPERBOLIC;
        if(options[3] == '1') flags |= SIMP_ID_COMPLEX;
        if(options[4] == '1') flags |= SIMP_ID_TRIG_CONSTANTS;
        if(options[5] == '1') flags |= SIMP_ID_TRIG_INV_CONSTANTS;
    }

    expression = parse_from_tok(input, &err);
    /*Fail silently because syntax is correct, but input might be bad. Let basic program handle error*/
    interface_assert(err == E_SUCCESS && expression != NULL, NULL);

    simplify(expression, flags);
    simplify_canonical_form(expression);

    write_to_tok(output, expression, &err);

    ast_Cleanup(expression);

    interface_assert(err == E_SUCCESS, NULL);

    success();
}

/*
    Syntax: EVAL,Y1,Y2
*/
void interface_Eval(arg_list *args) {
    pcas_ast_t *expression;
    pcas_error_t err;

    uint8_t *input, *output;
    unsigned input_len, output_len;

    interface_assert(args->amount >= 3, "Not enough arguments");

    tok_fix(&args->args[1], &args->arg_len[1]);
    tok_fix(&args->args[2], &args->arg_len[2]);

    input = args->args[1];
    output = args->args[2];

    input_len = args->arg_len[1];
    output_len = args->arg_len[2];

    interface_assert(tok_valid(input, input_len), "Not a valid input variable");
    interface_assert(tok_valid(output, output_len), "Not a valid output variable");

    expression = parse_from_tok(input, &err);
    /*Fail silently because syntax is correct, but input might be bad. Let basic program handle error*/
    interface_assert(err == E_SUCCESS && expression != NULL, NULL);

    simplify(expression, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL);
    eval(expression, EVAL_ALL);
    simplify(expression, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL | SIMP_EVAL | SIMP_LIKE_TERMS);
    simplify_canonical_form(expression);

    write_to_tok(output, expression, &err);

    ast_Cleanup(expression);

    interface_assert(err == E_SUCCESS, NULL);

    success();
}

/*
    Syntax: SUB,Y1,Y2,Str1,Str2
*/
void interface_Substitute(arg_list *args) {
    pcas_ast_t *expression, *sub_from_expr, *sub_to_expr;
    pcas_error_t err;

    uint8_t *input, *output, *sub_from, *sub_to;
    unsigned input_len, output_len, sub_from_len, sub_to_len;

    interface_assert(args->amount >= 5, "Not enough arguments");

    tok_fix(&args->args[1], &args->arg_len[1]);
    tok_fix(&args->args[2], &args->arg_len[2]);
    tok_fix(&args->args[3], &args->arg_len[3]);
    tok_fix(&args->args[4], &args->arg_len[4]);

    input = args->args[1];
    output = args->args[2];
    sub_from = args->args[3];
    sub_to = args->args[4];

    input_len = args->arg_len[1];
    output_len = args->arg_len[2];
    sub_from_len = args->arg_len[3];
    sub_to_len = args->arg_len[4];

    interface_assert(tok_valid(input, input_len), "Not a valid input variable");
    interface_assert(tok_valid(output, output_len), "Not a valid output variable");
    interface_assert(tok_valid(sub_from, sub_from_len), "Not a valid substitute from variable");
    interface_assert(tok_valid(sub_to, sub_to_len), "Not a valid substitute to variable");

    expression = parse_from_tok(input, &err);
    /*Fail silently because syntax is correct, but input might be bad. Let basic program handle error*/
    interface_assert(err == E_SUCCESS && expression != NULL, NULL);
    sub_from_expr = parse_from_tok(sub_from, &err);
    interface_assert(err == E_SUCCESS && sub_from_expr != NULL, NULL);
    sub_to_expr = parse_from_tok(sub_to, &err);
    interface_assert(err == E_SUCCESS && sub_to_expr != NULL, NULL);

    simplify(expression, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL);
    substitute(expression, sub_from_expr, sub_to_expr);
    simplify(expression, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL | SIMP_EVAL | SIMP_LIKE_TERMS);
    simplify_canonical_form(expression);

    write_to_tok(output, expression, &err);

    ast_Cleanup(expression);
    ast_Cleanup(sub_from_expr);
    ast_Cleanup(sub_to_expr);

    interface_assert(err == E_SUCCESS, NULL);

    success();
}

/*
    Syntax: EXP,Y1,Y2 or EXP,Y1,Y2,00

    All set by default.

    Boolean 1 = expand multiplication (A+X)(B+2)
    Boolean 2 = expand powers (1+A)^6
*/
void interface_Expand(arg_list *args) {
    pcas_ast_t *expression;
    pcas_error_t err;

    unsigned short flags = 0;

    uint8_t *input, *output;
    unsigned input_len, output_len;

    interface_assert(args->amount >= 3, "Not enough arguments");
    
    tok_fix(&args->args[1], &args->arg_len[1]);
    tok_fix(&args->args[2], &args->arg_len[2]);

    input = args->args[1];
    output = args->args[2];

    input_len = args->arg_len[1];
    output_len = args->arg_len[2];

    interface_assert(tok_valid(input, input_len), "Not a valid input variable");
    interface_assert(tok_valid(output, output_len), "Not a valid output variable");

    if(args->amount >= 4) {
        uint8_t *options;
        unsigned option_len;

        unsigned i = 0;

        options = args->args[3];
        option_len = args->arg_len[3];

        interface_assert(option_len == 2, "Wrong number of boolean options");

        for(i = 0; i < 2; i++)
            interface_assert(options[i] == '0' || options[i] == '1', "Boolean option must be 0 or 1");

        if(options[0] == '1') flags |= EXP_DISTRIB_NUMBERS | EXP_DISTRIB_MULTIPLICATION | EXP_DISTRIB_ADDITION;
        if(options[1] == '1') flags |= EXP_EXPAND_POWERS;
    } else {
        flags = EXP_DISTRIB_NUMBERS | EXP_DISTRIB_MULTIPLICATION | EXP_DISTRIB_ADDITION | EXP_EXPAND_POWERS;
    }

    expression = parse_from_tok(input, &err);
    /*Fail silently because syntax is correct, but input might be bad. Let basic program handle error*/
    interface_assert(err == E_SUCCESS && expression != NULL, NULL);

    simplify(expression, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL);
    expand(expression, flags);
    simplify(expression, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL | ((flags & EXP_DISTRIB_MULTIPLICATION) ? SIMP_LIKE_TERMS : 0) | SIMP_EVAL);
    simplify_canonical_form(expression);

    write_to_tok(output, expression, &err);

    ast_Cleanup(expression);

    interface_assert(err == E_SUCCESS, NULL);

    success();
}

bool valid_respect_to(uint8_t *symbol, unsigned symbol_len) {
    return symbol_len == 1 && symbol[0] >= 'A' && symbol[0] <= ('Z' + 1); /*Z + 1 is theta*/
}

/*
    Syntax: DERIV,Y1,Y2,X

    Solves derivative with respect to 3rd argument
*/
void interface_Derivative(arg_list *args) {
    pcas_ast_t *expression, *respect_to_expr;
    pcas_error_t err;

    uint8_t *input, *output, *respect_to;
    unsigned input_len, output_len, respect_to_len;

    interface_assert(args->amount >= 4, "Not enough arguments");

    tok_fix(&args->args[1], &args->arg_len[1]);
    tok_fix(&args->args[2], &args->arg_len[2]);

    input = args->args[1];
    output = args->args[2];
    respect_to = args->args[3];

    input_len = args->arg_len[1];
    output_len = args->arg_len[2];
    respect_to_len = args->arg_len[3];

    interface_assert(tok_valid(input, input_len), "Not a valid input variable");
    interface_assert(tok_valid(output, output_len), "Not a valid output variable");
    interface_assert(valid_respect_to(respect_to, respect_to_len), "Not a valid \"respect to\" variable");

    expression = parse_from_tok(input, &err);
    /*Fail silently because syntax is correct, but input might be bad. Let basic program handle error*/
    interface_assert(err == E_SUCCESS && expression != NULL, NULL);
    respect_to_expr = parse(respect_to, respect_to_len, str_table, &err);
    interface_assert(err == E_SUCCESS && respect_to_expr != NULL, NULL);

    simplify(expression, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL);
    derivative(expression, respect_to_expr, respect_to_expr);
    simplify(expression, SIMP_NORMALIZE | SIMP_COMMUTATIVE | SIMP_RATIONAL | SIMP_EVAL | SIMP_LIKE_TERMS);
    simplify_canonical_form(expression);

    write_to_tok(output, expression, &err);

    ast_Cleanup(expression);
    ast_Cleanup(respect_to_expr);

    interface_assert(err == E_SUCCESS, NULL);

    success();
}

bool interface_arg_equals(uint8_t *arg, unsigned arg_len, char *str2) {
    unsigned i;

    if(arg_len != strlen(str2))
        return false;

    for(i = 0; i < arg_len; i++)
        if(arg[i] != (uint8_t)str2[i])
            return false;

    return true;
}

void interface_Run() {
    uint8_t *ans;
    unsigned ans_len;

    arg_list args;

    ans = read_ans(&ans_len);

    if(parse_args(ans, ans_len, &args)) {

        if(interface_arg_equals(args.args[0], args.arg_len[0], "SIMP")) {
            interface_Simplify(&args);
        } else if(interface_arg_equals(args.args[0], args.arg_len[0], "EVAL")) {
            interface_Eval(&args);
        } else if(interface_arg_equals(args.args[0], args.arg_len[0], "SUB")) {
            interface_Substitute(&args);
        } else if(interface_arg_equals(args.args[0], args.arg_len[0], "EXP")) {
            interface_Expand(&args);
        } else if(interface_arg_equals(args.args[0], args.arg_len[0], "DERIV")) {
            interface_Derivative(&args);
        }

        id_UnloadAll();

        cleanup_args(&args);
    }

    free(ans);
}


/*Checks if Ans is trying to call a function of PCAS*/
bool interface_Valid() {
    uint8_t *ans;
    unsigned ans_len;

    arg_list args;

    bool valid = false;

    ans = read_ans(&ans_len);

    if(parse_args(ans, ans_len, &args)) {
        valid = interface_arg_equals(args.args[0], args.arg_len[0], "SIMP")
                || interface_arg_equals(args.args[0], args.arg_len[0], "EVAL")
                || interface_arg_equals(args.args[0], args.arg_len[0], "SUB")
                || interface_arg_equals(args.args[0], args.arg_len[0], "EXP")
                || interface_arg_equals(args.args[0], args.arg_len[0], "DERIV");

        cleanup_args(&args);
    }

    free(ans);

    return valid;
}

#else
typedef int make_iso_compilers_happy;
#endif