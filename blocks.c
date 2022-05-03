#include "blocks.h"
#include "stack.h"
#include "node.h"

void buildBasicBlocks(struct stack* s){
    int bufferSize = 2048;

    int blockCounter = 1; // track number of blocks
    char output[bufferSize]; // used to store all output
    char labelBuffer[bufferSize]; // used to store labels
    char instructionBuffer[bufferSize];
    char conditionalBuffer[bufferSize];
    char blockBuffer[bufferSize];

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
                    char newBlockLabel[bufferSize];
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
                    char newBlockLabel[bufferSize];
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
            char ifBlock[bufferSize];
            sprintf(ifBlock, "\nBB%d:\n", ifLabelID);
            sprintf(conditionalBuffer, "%s", pointer->equation); // split by newline
            delimited = strtok(conditionalBuffer, "\n");
            while(delimited != NULL){
                if(strncmp(delimited, "if(", 3) == 0){
                    char ifBuffer[bufferSize];
                    char ifBlockBuffer[bufferSize];
                    ifBlockBuffer[0] = '\0';
                    delimited = strtok(NULL, "\n"); // enter into the conditional block
                    while(strncmp(delimited, "}", 1) != 0){
                        sprintf(ifBuffer, "%s\n", delimited);
                        strcat(ifBlockBuffer, ifBuffer);
                        delimited = strtok(NULL, "\n");
                    }
                    strcat(ifBlock, ifBlockBuffer);
                    // Add jump to final label
                    char finalLabelJump[bufferSize];
                    sprintf(finalLabelJump, "\tgoto BB%d;\n", finalBlockID);
                    strcat(ifBlock, finalLabelJump);
                }
                delimited = strtok(NULL, "\n");
            }
            strcat(output, ifBlock);

            // Step Three: Create ELSE basic block
            char elseBlock[bufferSize];
            sprintf(elseBlock, "\nBB%d:\n", elseLabelID);
            sprintf(conditionalBuffer, "%s", pointer->equation); // split by newline
            delimited = strtok(conditionalBuffer, "\n");
            while(delimited != NULL){
                if(strncmp(delimited, "} else {", 8) == 0){
                    char elseBuffer[bufferSize];
                    char elseBlockBuffer[bufferSize];
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
                    char finalLabelJump[bufferSize];
                    sprintf(finalLabelJump, "\tgoto BB%d;\n", finalBlockID);
                    strcat(elseBlock, finalLabelJump);
                }
                // get contents of else
                delimited = strtok(NULL, "\n");
            }
            strcat(output, elseBlock);

            // Step Four: Create final basic block for post conditional
            char finalBlockLabel[bufferSize];
            blockCounter++;
            sprintf(finalBlockLabel, "\nBB%d:\n", finalBlockID);
            strcat(output, finalBlockLabel);
        }
        pointer = pointer->next;
    }
    // Show final output
    printf("%s\n", output);
}