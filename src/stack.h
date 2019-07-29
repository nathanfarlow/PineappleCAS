#ifndef STACK_H_
#define STACK_H_

/*the default stack size*/
#define STACK_START 10

typedef struct _Stack {
    unsigned int _max;
    unsigned int top;
    void **items;
} pcas_stack_t;

void stack_Create(pcas_stack_t *s);
void stack_Cleanup(pcas_stack_t *s);

void stack_Push(pcas_stack_t *s, void *item);
void *stack_Pop(pcas_stack_t *s);
void *stack_Peek(pcas_stack_t *s);

void stack_Clear(pcas_stack_t *s);

#endif