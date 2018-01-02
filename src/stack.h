#ifndef _STACK_H_
#define _STACK_H_

//the default stack size
#define STACK_START 10

typedef struct _Stack {
    unsigned int _max;
    unsigned int top;
    void **items;
} stack_t;

void stack_Create(stack_t *s);
void stack_Cleanup(stack_t *s);

void stack_Push(stack_t *s, void *item);
void *stack_Pop(stack_t *s);
void *stack_Peek(stack_t *s);

void stack_Clear(stack_t *s);

#endif