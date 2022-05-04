#include "stack.h"
#include "node.h"
#include "latency.h"

//#define DEBUG
#define BUFFERLEN 2048

// helper function to get the number of live variables in a given node
int getNumOfLiveVars(struct node* n){
    int num = 0;
    char* delimited = strtok(n->liveVars, ",");
    while(delimited != NULL){
        num++;
        delimited = strtok(NULL, ",");
    }
    #ifdef DEBUG
    printf("Number of live variables in node %s: %d\n", n->nodeName, num);
    #endif
    return num;
}

char* getLiveVariables(char* liveVars){
    // parse live variables, ignore numbers
    char* newLiveVars = malloc(sizeof(char) * 100);
    char* delimited = strtok(liveVars, ",");
    while(delimited != NULL){
        if(strstr(newLiveVars, delimited) == NULL){
            // printf("new one\n");
            // printf("%d\n", isdigit(*delimited));
            if(!isdigit(*delimited)){
                strcat(newLiveVars, delimited);
                strcat(newLiveVars, " ");
            }
        }
        delimited = strtok(NULL, ",");
    }
    #ifdef DEBUG
    printf("Live variables minus numbers %s\n", newLiveVars);
    #endif
    return newLiveVars;
}

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

// Helper function to show dependencies
void printGraphDependencies(struct graphNode** graph, int graphSize){
    struct graphNode* pointer;
    printf("graph size:%d\n", graphSize);
    printf("test graph size: %lf\n", (double)(sizeof(graph) / sizeof(struct graphNode*)));
    for(int i = 0; i < graphSize; i++){
        pointer = graph[i];
        printf("Variable %s: ", pointer->nodeName);
        printf("Variable has %d edges, ", pointer->edgeCount);
        for(int j = 0; j < pointer->edgeCount; j++){
            printf("%s,", pointer->edges[i]->nodeName);
        }
        printf("\n");
    }
}

// checks to see if a node shares an edge with another node
bool isDependent(struct graphNode* check, struct graphNode* compare){
    for(int i = 0; i < check->edgeCount; i++){
        if(check->edges[i] == compare){
            return true;
        }
    }
    return false;
}

// gets total latency of conditional block
int getConditionalLatency(struct node* instruction){
    int latency = 2; // ? operator has a latency of 2
    char* delimited = strtok(instruction->equation, "\n");
    while(delimited != NULL){
        if(!strstr(delimited, "}") && !strstr(delimited, "{")){
            // contents will ever only be assignment, which carries a latency of 2
            latency += 2;
        }
        delimited = strtok(NULL, "\n");
    }
    return latency;
}

// checks for operator within instruction, returns total latency
int getInstructionLatency(struct node* instruction){
    int latency = 2; // will always involve =, with a latency of 2
    //printf("current eq: %s\n", instruction->equation);
    // check for operators
    if(strstr(instruction->equation, "+") || strstr(instruction->equation, "-")){
        latency += 1;
    }else if(strstr(instruction->equation, "if")){
        // Handles conditional case
        latency = getConditionalLatency(instruction);
    }else if(strstr(instruction->equation, "*") || strstr(instruction->equation, "/")){
        latency += 4;
    }else if(strstr(instruction->equation, "**")){
        latency += 8;
    }
    return latency;
}

void calculateLatency(struct stack* s){
    int numVars = getNumOfVariables(s);
    int graphSize = numVars;
    char* uniqueVars = getUniqueVariables(s);

    int totalCycles = 0; // track total duration in cycles
    int currentCycle = 0; // track which cycle the program is currently on

    int currentInstructionStart = 0;
    int currentInstructionStop = 0;
    int nextInstructionStart = 0;
    int nextInstructionStop = 0;
    int instructionCount = 1;

    // create graph
    struct graphNode** graph = makeGraph(s, uniqueVars, numVars);
    //printGraphDependencies(graph, numVars);
    char instructionBuffer[BUFFERLEN];
    int currentInstructionLatency;


    struct node* pointer = s->bottom;
    while(pointer->next != NULL){
        //printf("Current pointer val %s\n", pointer->equation);
        currentInstructionLatency = getInstructionLatency(pointer);
        currentInstructionStart = nextInstructionStart; // starts at current cycle
        //currentInstructionStop = currentInstructionStart + currentInstructionLatency;
        currentInstructionStop = ((currentInstructionStart + currentInstructionLatency) > nextInstructionStop) ? (currentInstructionStart + currentInstructionLatency) : nextInstructionStop;

        // To fix instruction equation problems
        char eq[BUFFERLEN];
        sprintf(eq, "%s%s", pointer->nodeName, pointer->equation);
        eq[strlen(eq) - 1] = 0;
        printf("Instruction #%d (%s) - Start: C%d, Stop: C%d\n", instructionCount, eq, currentInstructionStart, currentInstructionStop);
        //printf("latency #%d: %d\n", instructionCount, currentInstructionLatency);

        // check to see dependent instructions
        struct graphNode* check = findGraphNode(graph, numVars, pointer->nodeName);
        if(pointer->next != NULL){
            struct graphNode* compare = findGraphNode(graph, numVars, pointer->next->nodeName);
            if(isDependent(check, compare)){
                // If next instruction is dependent on previous instruction...
                int dependentLatency = getInstructionLatency(pointer->next);
                
                nextInstructionStart = currentInstructionStop;
                nextInstructionStop = nextInstructionStart + dependentLatency;
                //printf("Next instruction (dependent) - Start: C%d, Stop: C%d\n", nextInstructionStart, nextInstructionStop);
            }else{
                // If instruction is NOT dependent on previous instruction...
                nextInstructionStart = currentInstructionStart + 1;
                nextInstructionStop = nextInstructionStart + getInstructionLatency(pointer->next);
                //printf("Next instruction - Start: C%d, Stop: C%d\n", nextInstructionStart, nextInstructionStop);
            }
            printf("\n");
        }
        currentCycle = currentInstructionStop;
        instructionCount++;
        pointer = pointer->next;
    }

    printf("Total cycles to run program: %d\n", currentCycle);
}