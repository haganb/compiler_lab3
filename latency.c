#include "stack.h"
#include "node.h"
#include "latency.h"

#define DEBUG
#define BUFFERLEN 2048

// helper function to calculate the number of unique variables used in a stack
int getNumOfVariables(struct stack* s){
    char buffer[BUFFERLEN];
    int num = 0;

    struct node* pointer = s->bottom;
    while(pointer->next != NULL){
        // check if variable has been checked before
        if(strstr(buffer, pointer->nodeName) == NULL){
            strcat(buffer, pointer->nodeName); // add node name to buffer so it isnt added again
            num++;
        }
        pointer = pointer->next;
    }
    #ifdef DEBUG
    printf("Number of unique variables: %d\n", num);
    #endif
    return num;
}

// helper function to get an actual string of all unique variable names, seperated by a space
char* getUniqueVariables(struct stack* s){
    char* uniqueVariables = malloc(sizeof(char) * BUFFERLEN);
    struct node* pointer = s->bottom;
    while(pointer->next != NULL){
        // check if variable has been checked before
        if(strstr(uniqueVariables, pointer->nodeName) == NULL){
            //sprintf(uniqueVariables, "%s, ", pointer->nodeName); // add node name to buffer so it isnt added again
            strcat(uniqueVariables, pointer->nodeName);
            strcat(uniqueVariables, " ");
        }
        pointer = pointer->next;
    }
    //printf("%s\n", uniqueVariables);
    #ifdef DEBUG
    printf("Unique variables: %s\n", uniqueVariables);
    #endif
    return uniqueVariables;
}

// create a new graphNode
struct graphNode* createGraphNode(struct stack* s, char* nodeName){
    struct graphNode* newGraphNode = malloc(sizeof(struct graphNode));
    newGraphNode->nodeName = nodeName;

    // allocate and assign values for edge attributes
    newGraphNode->edges = malloc((sizeof(struct graphNode)) * 50);
    newGraphNode->edgeCount = 0;

    // set other default values
    newGraphNode->allocatedRegister = 0; // defaults to having no allocated register

    // calculate the life of the new graph node
    int liveStart = 0;
    int liveEnd = 0;
    int mostRecentInstructionNum; // keeps track of the most recent instruction number that a variable was seen
    int instructionNum = 1;

    // iterate through stack, checking live vars for each instruction
    struct node* pointer = s->bottom;
    while(pointer->next != NULL){
        // if liveVar attribute contains node name
        // printf("pre parsed live variables: %s\n", pointer->liveVars);
        // char* currentLiveVars = getLiveVariables(pointer->liveVars);
        // printf("post parsed live variables: %s\n", currentLiveVars);
        if(strstr(pointer->liveVars, nodeName) != NULL){
            if(liveStart == 0){
                liveStart = instructionNum;
            }
            mostRecentInstructionNum = instructionNum;
        }
        instructionNum++;
        pointer = pointer->next;
    }
    liveEnd = mostRecentInstructionNum + 1; // NOTE: Keep track of this, could be wrong !
    newGraphNode->liveStart = liveStart;
    newGraphNode->liveEnd = liveEnd;
    return newGraphNode;
}

// find specific graphNode within graph
struct graphNode* findGraphNode(struct graphNode** graph, int graphSize, char* nodeName){
    for(int i = 0; i < graphSize; i++){
        if(strcmp(graph[i]->nodeName, nodeName) == 0){
            return graph[i];
        }
    }

    // if node cannot be found, or if node is repesented by a number
    // NOTE: this is not a good way to do this but its much easier than adding checks for number values
    return NULL;
}

// create graph structure using variables
struct graphNode** makeGraph(struct stack* s, char* varList, int numOfVars){
    struct graphNode** graph = malloc(sizeof(struct graphNode*) * numOfVars);

    int nodeCounter = 0;
    char* delimited = strtok(varList, " "); // seperate variables
    // iterate through all unique variable names, create new node for them
    while(delimited != NULL){
        // should account for copy variables
        struct graphNode* newNode = createGraphNode(s, delimited); 
        graph[nodeCounter] = newNode;
        nodeCounter++;
        delimited = strtok(NULL, " ");
    }

    #ifdef DEBUG
    printf("Done making nodes...\n");
    #endif

    // set all edge values in graph
    for(int i = 0; i < numOfVars; i++){
        struct graphNode* pointer1 = graph[i];
        
        // compare node against all nodes after it
        for(int registerNum = i + 1; registerNum < numOfVars; registerNum++){
            // needs to set edge if nodes have overlapping lives
            struct graphNode* pointer2 = graph[registerNum];

            // nodes overlap if:
            // if pointer1 lives after or at the same time as pointer2, and pointer1 starts before pointer 2 ends
            bool case1 = (pointer1->liveStart >= pointer2->liveStart) && (pointer1->liveStart < pointer2->liveEnd);
            // if pointer1 lives before or at the same time as pointer2, and pointer1 ends after pointer 2 starts
            bool case2 = (pointer1->liveStart <= pointer2->liveStart) && (pointer1->liveEnd > pointer2->liveStart); 

            if(case1 || case2){
                // add edge from 1 to 2
                pointer1->edges[pointer1->edgeCount] = pointer2;
                pointer1->edgeCount++;

                // add edge from 2 to 1
                pointer2->edges[pointer2->edgeCount] = pointer1;
                pointer2->edgeCount++;
            }
        }
    }
    return graph;
}


void calculateLatency(struct stack* s){
    int numVars = getNumOfVariables(s);
    int graphSize = numVars;
    char* uniqueVars = getUniqueVariables(s);

    // create graph
    #ifdef DEBUG
    printf("Making graph...\n");
    #endif

    struct graphNode** graph = makeGraph(s, uniqueVars, numVars);
    
    #ifdef DEBUG
    printf("Graph made...\n");
    #endif
}