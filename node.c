#include "node.h"
//#define DEBUG

// ***********************  MAIN LINKED LIST FUNCTIONS *********************** 
// Helper function for creating new node
struct node* newNode(char* nodeName, int nodeVal){
    #ifdef DEBUG
    printf("Creating new node with name %s and val %d\n", nodeName, nodeVal);
    #endif

    struct node* new_node = (struct node*)malloc(sizeof(struct node));
    sscanf(nodeName, "%s", new_node->nodeName);
    new_node->nodeVal = nodeVal;
    new_node->next = NULL;

    return new_node;
}

struct node* addNode(struct node* head, char* nodeName, int nodeVal){
    struct node* pointer = head;
    while(pointer->next != NULL){
        pointer = pointer->next;
    }
    struct node* n = newNode(nodeName, nodeVal);
    pointer->next = n;
    return n;
}

struct node* findNode(char* nodeName, int nodeVal, struct node* n){
    if(strcmp(n->nodeName, nodeName) == 0){
        // If node is found
        return n;
    }else if(n->next == NULL){
        // If you reach end of chain and don't find it, make it and append
        return addNode(n, nodeName, nodeVal);
    }
    else{
        // Move to next layer
        return findNode(nodeName, nodeVal, n->next);
    }
}


// *********************** HELPER FUNCTIONS *********************** 
// Print name and value of all nodes in list
void printList(struct node* head){
    printf("Linked list:\n");
    struct node* pointer = head;
    while(pointer->next != NULL){
        printf("Node %s with value %d\n", pointer->nodeName, pointer->nodeVal);
        pointer = pointer->next;
    }
    printf("\n");
}

// Delete all nodes in linked list to prevent memory leaks
void deleteNodes(struct node* head){
    #ifdef DEBUG
    printf("Freeing all nodes...\n");
    #endif
    if(head != NULL){
        struct node* pointer = head->next;
        while(pointer != NULL){
            struct node* placeholder = pointer->next;
            free(pointer);
            pointer = placeholder;
        }
    }else{
        free(head);
    }
    

    #ifdef DEBUG
    printf("Nodes freed...\n");
    #endif
}