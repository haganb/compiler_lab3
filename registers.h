#ifndef _REGISTERS_H_
#define _REGISTERS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

struct graphNode{
    char* nodeName;
    int allocatedRegister; // id for whatever register is currently allocated for

    struct graphNode** edges; // keep track of which nodes are dependent on other nodes
    int edgeCount; // total number of edges for a given node

    int liveStart; // corresponds to which instruction variable is first referenced in
    int liveEnd; // corresponds to which instruction variable is last referenced in

    
}graphNode;

struct graph{
    struct graphNode* nodes;
    int size;
}graph;

void allocateRegisters(struct stack* s);

#endif