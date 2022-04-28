#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct stack{
    struct node* top;
    struct node* bottom;
    int height;
}stack;

/* actual stack functions for pushing and popping */
struct node* push(struct stack* s, char* nodeName, int nodeVal);
struct node* pop(struct stack* s);

// helper functions
void printStack(struct stack* s);
void deleteStack(struct stack* s);