#include "blocks.h"
#include "stack.h"
#include "node.h"

void buildBasicBlocks(struct stack* s){
    int blockCounter = 1; // track number of blocks
    char output[1024]; // used to store all output
    char labelBuffer[1024]; // used to store labels
    char instructionBuffer[1024];
    char conditionalBuffer[1024];
    char blockBuffer[1024];

    // Add first block label
    sprintf(labelBuffer, "BB%d:\n", blockCounter); 
    strcat(output, labelBuffer);

    // Traverse through stack to build output
    struct node* pointer = s->bottom;

    while(pointer->next != NULL){
        // No new block needed if three address code does not have conditional
        if(strstr(pointer->equation, "if(") == NULL){
            sprintf(instructionBuffer, "\t%s%s", pointer->nodeName, pointer->equation); // load instruction into buffer
            strcat(output, instructionBuffer); // append to output
        }else{
            int ifLabelID = 0; // corresponds to block ID of IF block
            int elseLabelID = 0; // corresponds to block ID of ELSE block

            // Step One: Re-write if/else statement to include GOTO statements with block IDS
            sprintf(conditionalBuffer, "%s", pointer->equation); 
            char *delimited = strtok(conditionalBuffer, "\n");
            while(delimited != NULL){
                // handle contents of if
                if(strncmp(delimited, "if(", 3) == 0){
                    sprintf(blockBuffer, "\t%s", delimited);

                    // add new block label
                    char newBlockLabel[1024];
                    blockCounter++;
                    ifLabelID = blockCounter;
                    sprintf(newBlockLabel, "\n\t\tgoto BB%d;\n", ifLabelID);
                    strcat(blockBuffer, newBlockLabel);
                    strcat(output, blockBuffer);
                }
                // handle contents of else
                else if(strncmp(delimited, "} else {", 8) == 0){
                    sprintf(blockBuffer, "\t%s", delimited);

                    // add new block label
                    char newBlockLabel[1024];
                    blockCounter++;
                    elseLabelID = blockCounter;
                    sprintf(newBlockLabel, "\n\t\tgoto BB%d;\n\t}", elseLabelID);
                    strcat(blockBuffer, newBlockLabel);
                    strcat(output, blockBuffer);
                }
                delimited = strtok(NULL, "\n");
            }

            int finalBlockID = elseLabelID + 1; // need this id for proper labelling, final block ID should always come 1 after the else block ID

            // Step Two: Create IF basic block
            char ifBlock[1024];
            sprintf(ifBlock, "\nBB%d:\n", ifLabelID);
            sprintf(conditionalBuffer, "%s", pointer->equation); // split by newline
            delimited = strtok(conditionalBuffer, "\n");
            while(delimited != NULL){
                if(strncmp(delimited, "if(", 3) == 0){
                    char ifBuffer[1024];
                    char ifBlockBuffer[1024];
                    ifBlockBuffer[0] = '\0';
                    delimited = strtok(NULL, "\n"); // enter into the conditional block
                    while(strncmp(delimited, "}", 1) != 0){
                        sprintf(ifBuffer, "%s\n", delimited);
                        strcat(ifBlockBuffer, ifBuffer);
                        delimited = strtok(NULL, "\n");
                    }
                    strcat(ifBlock, ifBlockBuffer);
                    // Add jump to final label
                    char finalLabelJump[1024];
                    sprintf(finalLabelJump, "\tgoto BB%d;\n", finalBlockID);
                    strcat(ifBlock, finalLabelJump);
                }
                delimited = strtok(NULL, "\n");
            }
            strcat(output, ifBlock);

            // Step Three: Create ELSE basic block
            char elseBlock[1024];
            sprintf(elseBlock, "\nBB%d:\n", elseLabelID);
            sprintf(conditionalBuffer, "%s", pointer->equation); // split by newline
            delimited = strtok(conditionalBuffer, "\n");
            while(delimited != NULL){
                if(strncmp(delimited, "} else {", 8) == 0){
                    char elseBuffer[1024];
                    char elseBlockBuffer[1024];
                    elseBlockBuffer[0] = '\0';
                    delimited = strtok(NULL, "\n"); // enter into the conditional block
                    while(strncmp(delimited, "}", 1) != 0){
                        sprintf(elseBuffer, "%s\n", delimited);
                        //printf("%s\n", delimited);
                        strcat(elseBlockBuffer, elseBuffer);
                        delimited = strtok(NULL, "\n");
                    }
                    strcat(elseBlock, elseBlockBuffer);
                    // Add jump to final label
                    char finalLabelJump[1024];
                    sprintf(finalLabelJump, "\tgoto BB%d;\n", finalBlockID);
                    strcat(elseBlock, finalLabelJump);
                }
                // get contents of else
                delimited = strtok(NULL, "\n");
            }
            strcat(output, elseBlock);

            // Step Four: Create final basic block for post conditional
            char finalBlockLabel[1024];
            blockCounter++;
            sprintf(finalBlockLabel, "\nBB%d:\n", finalBlockID);
            strcat(output, finalBlockLabel);
        }
        pointer = pointer->next;
    }
    // Show final output
    printf("%s\n", output);
}