%{
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "node.h"
#include "stack.h"
#include "blocks.h"
#include "latency.h"

/* Compiler Design
University of Delaware Spring 2022
Lab 3: Calculator Compiler Middle End
Hagan Beatson */

#define DEBUG /* for debugging purposes */
#define NAMESIZE 32  
#define BUFFERSIZE 1032 // global constant for buffer arrays

// Global variables, etc
extern FILE *yyin;      // file to load equations into
int lineNum = 1;        // keeps track of line numbers
int tmpNum = 1; // pointer to keep track of placeholder nodes
struct stack* STACK; // global stack structure
struct node* HEAD; // global head node
char inputVariables[BUFFERSIZE]; // array to keep track of which variables are user defined and which are not
int BLOCK_COUNTER = 1;

// Function declarations
void addNewInputVariable(char* name);

void yyerror(char *ps, ...) { /* need this to avoid link problem */
	printf("%s\n", ps);
}

%}

%union {
    int d; /* value */
    char name[32]; /* for variables */
    struct node* node;
}

%token <d> NUM 
%token <name> VAR
%token '(' ')'
%left '+' '-' '*' '/'
%right '=' EXP '!' '?' '\n'

//%type <d> expression factor term statement
%type <node> expression
%start infix

%%
infix: 
    infix equation '\n' |;

equation:
    expression {}
expression :
    NUM{
        $$ = newNode("Tmp", $1);
        sprintf($$->nodeName, "%d", $1);
    }
    | '(' expression ')' {
        $$ = $2;
    }
    | '(' expression '(' {
        printf("ERROR, bad parentheses\n");
    }
    | ')' expression ')' {
        printf("ERROR, bad parentheses\n"); // no need for debug flags, should always print
    }
    | expression '+' expression {
        int output = $1->nodeVal + $3->nodeVal; // result of operation
        $$ = push(STACK, "Tmp", output); // push tmp node to stack
        sprintf($$->nodeName, "Tmp%d", tmpNum); // load tmpname with corresponding counter into output nodename
        tmpNum++; // increment tmp counter
        sprintf($$->equation, " = %s + %s;\n", $1->nodeName, $3->nodeName); // build expression string for output node
        
        // create attribute for live variables, have to ignore numbers
        sprintf($$->liveVars, "%s,%s,%s", $$->nodeName, $1->nodeName, $3->nodeName); // needed for task #2;
    }
    | expression '-' expression {
        // same as + case, just change operation
        int output = $1->nodeVal - $3->nodeVal;
        $$ = push(STACK, "Tmp", output);
        sprintf($$->nodeName, "Tmp%d", tmpNum);
        tmpNum++;
        sprintf($$->equation, " = %s - %s;\n", $1->nodeName, $3->nodeName);
        sprintf($$->liveVars, "%s,%s,%s", $$->nodeName, $1->nodeName, $3->nodeName); // needed for task #2;
    }
    | expression '*' expression {
        // same as + case, just change operation
        int output = $1->nodeVal * $3->nodeVal;
        $$ = push(STACK, "Tmp", output);
        sprintf($$->nodeName, "Tmp%d", tmpNum);
        tmpNum++;
        sprintf($$->equation, " = %s * %s;\n", $1->nodeName, $3->nodeName);
        sprintf($$->liveVars, "%s,%s,%s", $$->nodeName, $1->nodeName, $3->nodeName); // needed for task #2;
    }
    | expression '/' expression {
        // same as + case, just change operation
        int output = $1->nodeVal / $3->nodeVal;
        $$ = push(STACK, "Tmp", output);
        sprintf($$->nodeName, "Tmp%d", tmpNum);
        tmpNum++;
        sprintf($$->equation, " = %s / %s;\n", $1->nodeName, $3->nodeName);
        sprintf($$->liveVars, "%s,%s,%s", $$->nodeName, $1->nodeName, $3->nodeName); // needed for task #2;
    }
    | expression EXP expression {
        // same as + case, just change operation
        int output = $1->nodeVal;
        for(int i = 0; i < $3->nodeVal; i++){
            output *= $1->nodeVal;
        }
        $$ = push(STACK, "Tmp", output);
        sprintf($$->nodeName, "Tmp%d", tmpNum);
        tmpNum++;
        sprintf($$->equation, " = %s ** %s;\n", $1->nodeName, $3->nodeName);
        sprintf($$->liveVars, "%s,%s,%s", $$->nodeName, $1->nodeName, $3->nodeName); // needed for task #2;
    }
    | expression '?' expression {
        int output;
        if($1->nodeVal == 0){
            output = 0;
        }else{
            output = $3->nodeVal;
        } 
        
        // first part of conditional statement
        char exp[BUFFERSIZE];
        sprintf(exp, "if(%s){\n", $1->nodeName);

        // fill out first "if" conditional with proper expression
        while(STACK->top != $1){
            struct node* n = pop(STACK);
            char exp_if[BUFFERSIZE];
            sprintf(exp_if, "\t%s%s", n->nodeName, n->equation);
            strcat(exp, exp_if);
        }
        $$ = push(STACK, "Tmp", output); // pushes to stack
        sprintf($$->nodeName, "Tmp%d", tmpNum); // give temporary value for node
        tmpNum++;

        // fill out "else" conditional
        char exp_else[BUFFERSIZE];
        sprintf(exp_else, "\t%s = %s;\n} else {\n\t%s = 0;\n}\n", $$->nodeName, $3->nodeName, $$->nodeName);
        strcat(exp, exp_else);
        sprintf($$->equation, "%s", exp);
        sprintf($$->liveVars, "%s,%s,%s", $$->nodeName, $1->nodeName, $3->nodeName); // needed for task #2;



    }
    | '!' expression {
        // same as + case, just change operation
        int output;
        if($2->nodeVal == 0){
            output = 1;
        }else{
            output = 0;
        } 
        $$ = push(STACK, "Tmp", output);
        sprintf($$->nodeName, "Tmp%d", tmpNum);
        tmpNum++;
        sprintf($$->equation, " = %d;\n", output);
        sprintf($$->liveVars, "%s", $$->nodeName); // needed for task #2;
    }
    | VAR '=' expression {
        struct node* n = findNode((char*)$1, $3->nodeVal, HEAD); // creates new node if necessary
        n->nodeVal = $3->nodeVal; // re-assigns node value
        $$ = push(STACK, n->nodeName, n->nodeVal);
        sprintf($$->equation, " = %s;\n", $3->nodeName);
        sprintf($$->liveVars, "%s,%s", $$->nodeName, $3->nodeName); // needed for task #2;
    }
    | VAR {
        struct node* n = findNode((char*)$1, 0, HEAD); // creates new node if necessary (shouldn't have to in this case)
        // Add variable to input variables for task #3
        addNewInputVariable(n->nodeName);
        $$ = newNode(n->nodeName, n->nodeVal); // add new node
    }
%%

// Helper function to add input variable names to global list with duplicate protection
void addNewInputVariable(char* name){
    if(!strstr(inputVariables, name)){
        strcat(inputVariables, name);
        strcat(inputVariables, " ");
    }
}

// Main function
int main(int argc, char* argv[]){
    HEAD = (struct node*)malloc(sizeof(struct node*)); // instantiate global HEAD node
    STACK = (struct stack*)malloc(sizeof(struct stack*)); // instantiate global STACK
    STACK->top = NULL;
    STACK->bottom = NULL;
    STACK->height = 0;
    
    // Open file 
    yyin = fopen("equation.txt", "r"); // file should always be equation.txt
    printf("\n"); // TODO: Program segfaults without this. What is going on?
    yyparse();
    //printStack(STACK);

    // Task #1
    printf("\n****************************\n");
    printf("Generating Basic Blocks (Task #1):\n");  
    buildBasicBlocks(STACK);

    // Task #2
    printf("****************************\n");
    printf("Performance Modelling (Task #2):\n");
    calculateLatency(STACK);

    printf("****************************\n");
    // Complete
    fclose(yyin);
    return 0;
}