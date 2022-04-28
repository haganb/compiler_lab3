#include "stack.h"
#include "node.h"
#include "registers.h"

#define MAXNODES 100
#define BUFFERLEN 2048
//#define DEBUG

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

// check to see if inputted graph is 4-colorable, returns false otherwise
bool isFourColorable(struct graphNode** graph, int graphSize){
    // loop through each graphNode in graph
    for(int i = 0; i < graphSize; i++){
        // For each register (starting from 1, ending at 4)
        for(int registerNum = 0; registerNum < 4; registerNum++){

            bool flag = true;
            // check all graphNodes that share edge with given graphNode
            for(int nodeNeighbors = 0; nodeNeighbors < graph[i]->edgeCount; nodeNeighbors++){
                // if register is already allocated
                if(graph[i]->edges[nodeNeighbors]->allocatedRegister == (registerNum + 1)){
                    flag = false; // not 4-colorable as register has already been assigned
                }
            }

            // if 4-colorable, store which register is being allocated for given variable
            if(flag){
                graph[i]->allocatedRegister = (registerNum + 1); // set allocated register value
            }
        }
        // if no register is allocated
        if(graph[i]->allocatedRegister == 0){
            return false;
        }
    }
    return true;
}

struct graphNode** splitGraph(struct graphNode** graph, int graphSize){}

// main method to combine all other methods
void allocateRegisters(struct stack* s){
    // get total number of variables in stack
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
    
    // check if graph if 4-colorable; if not, modify
    while(!isFourColorable(graph, graphSize)){
        // if the graph is not 4-colorable, we should start by finding which variable lives the longest
        
        int longestDuration = 0; // difference between start and end of variables liveness
        int targetNodeIndex = 0; // index for variable with longest duration

        for(int i = 0; i < graphSize; i++){
            int currentDuration = graph[i]->liveEnd - graph[i]->liveStart;
            if(currentDuration > longestDuration){
                longestDuration = currentDuration;
                targetNodeIndex = graphSize - i - 1;
            }
        }

        // retrieve variable with longest duration
        struct graphNode* targetNode = graph[targetNodeIndex];
        printf("%s\n", targetNode->nodeName);

        // get live variables for the final instruction of targetNodes lifecycle
        int endInstructionNum = targetNode->liveEnd - 1;
        int instructionCount = 1;

        // iterate thru struct to find final instruction
        struct node* pointer = s->bottom;
        while(instructionCount != endInstructionNum){
            instructionCount++;
            pointer = pointer->next;
        }
        char* liveVars = strtok(pointer->liveVars, "");

        // iterate thru live variables for final instruction in variables life cycle
        while(liveVars){
            for(int i = 0; i < graphSize; i++){
                if(strcmp(graph[i]->nodeName, liveVars)){
                    // Create a new node with a temp name
                    char newNodeName[BUFFERLEN];
                    strcat(newNodeName, graph[i]->nodeName);
                    strcat(newNodeName, "_split "); // signifies that a new split version of previous variable was added to rebalance graph
                    // a space character is added to keep it consistent with uniqueVars syntax
                    // add new variable name to unique variable list for future graph re-draw
                    strcat(uniqueVars, newNodeName);
                }
            }
            liveVars = strtok(NULL, " ");
        }

        // re-make graph with updated values
        graph = makeGraph(s, uniqueVars, numVars);
        graphSize++;
    }

    // keeps track of which variable is currently loaded in registers 1 thru 4
    // default set to 0, but should change by checking each live variable per instruction
    char reg1[BUFFERLEN] = "0";
    char reg2[BUFFERLEN] = "0";
    char reg3[BUFFERLEN] = "0";
    char reg4[BUFFERLEN] = "0";

    // iterate through stack
    struct node* pointer = s->bottom;
    while(pointer->next != NULL){
        #ifdef DEBUG
        printf("*********\n");
        printf("Current node expression: %s%s\n", pointer->nodeName, pointer->equation);
        printf("Current node live variable list: %s\n", pointer->liveVars);
        printf("*********\n");
        #endif

        // iterate through all live variables for each instruction
        char* liveVars = strtok(pointer->liveVars, ",");
        while(liveVars != NULL){
            // find node corresponding to current live variable
            struct graphNode* currentNode = findGraphNode(graph, graphSize, liveVars);
            if(currentNode != NULL){
                #ifdef DEBUG
                printf("current live variable: %s\n", currentNode->nodeName);
                #endif
                // if you find the node...
                char regBuffer[BUFFERLEN];
                int index = currentNode->allocatedRegister - 1; // 0 indexing
                // load proper register 
                switch(index){
                    // for each case, we load the current variable name into the regBuffer, and then load the new variable into the respective
                    // register value
                    case 0:
                        sprintf(regBuffer, "%s", reg1);
                        sprintf(reg1, "%s", currentNode->nodeName);
                        break;
                    case 1:
                        sprintf(regBuffer, "%s", reg2);
                        sprintf(reg2, "%s", currentNode->nodeName);
                        break;
                    case 2:
                        sprintf(regBuffer, "%s", reg3);
                        sprintf(reg3, "%s", currentNode->nodeName);
                        break;
                    case 3:
                        sprintf(regBuffer, "%s", reg1);
                        sprintf(reg4, "%s", currentNode->nodeName);
                        break;
                }
                if(strcmp(regBuffer, currentNode->nodeName) != 0){
                    // if register has not yet had a variable set to it
                    if(strcmp(regBuffer, "0") != 0){
                        printf("%s=r%d\n", regBuffer, currentNode->allocatedRegister);
                    }else{
                        printf("r%d=%s\n", currentNode->allocatedRegister, currentNode->nodeName);
                    }
                }
            
            }
            liveVars = strtok(NULL, ",");
        }
        pointer = pointer->next;
    }

    // retrieve final register values
    struct graphNode* reg1Node = findGraphNode(graph, graphSize, reg1);
    struct graphNode* reg2Node = findGraphNode(graph, graphSize, reg2);
    struct graphNode* reg3Node = findGraphNode(graph, graphSize, reg3);
    struct graphNode* reg4Node = findGraphNode(graph, graphSize, reg4);

    // print final register values
    printf("Final variable/register pairings:\n");
    printf("%s=r1\n", reg1Node->nodeName);
    printf("%s=r2\n", reg2Node->nodeName);
    printf("%s=r3\n", reg3Node->nodeName);
    printf("%s=r4\n", reg4Node->nodeName);
}