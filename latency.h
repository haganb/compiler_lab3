#ifndef _LATENCY_H_
#define _LATENCY_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

struct graphNode{
    char* nodeName;

    struct graphNode** edges; // keep track of which nodes are dependent on other nodes
    int edgeCount; // total number of edges for a given node

    int liveStart; // corresponds to which instruction variable is first referenced in
    int liveEnd; // corresponds to which instruction variable is last referenced in

    
}graphNode;

struct graph{
    struct graphNode* nodes;
    int size;
}graph;

#endif