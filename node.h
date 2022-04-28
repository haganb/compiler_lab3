#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct node{
        char nodeName[32];
        int nodeVal;
        char equation[1000];
        struct node* next;
        char liveVars[1000];
}node;

/* linked list function declarations */
struct node* newNode(char* nodeName, int nodeVal);
struct node* addNode(struct node* head, char* nodeName, int nodeVal);
struct node* findNode(char* nodeName, int nodeVal, struct node* n);


// Helper Functions
void deleteNodes(struct node* head);
void printList(struct node* head);