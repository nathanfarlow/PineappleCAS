#include "stack.h"

#include <stdlib.h>

void stack_Create(stack_t *s) {
    s->top = 0;
    s->_max = STACK_START;
    s->items = malloc(STACK_START * sizeof(void*));
}

void stack_Cleanup(stack_t *s) {
    free(s->items);
}

void stack_Push(stack_t *s, void *item) {
    if (s->top >= s->_max) {
        s->_max *= 2;
        s->items = realloc(s->items, s->_max * sizeof(void*));
    }

    s->items[s->top++] = item;
}

void *stack_Pop(stack_t *s) {
    if (s->top <= 0) {
        s->top = 0;
        return NULL;
    }

    return s->items[--s->top];
}

void *stack_Peek(stack_t *s) {
    if (s->top <= 0) {
        s->top = 0;
        return NULL;
    }

    return s->items[s->top - 1];
}

void stack_Clear(stack_t *s) {
    s->top = 0;
}