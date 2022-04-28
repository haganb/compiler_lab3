%{
#include <stdlib.h> /* for atoi call */
#include <string.h>
#include "infix.tab.h" 

//#define DEBUG /* for debuging: print tokens and their line numbers */

YYSTYPE yylval; /* for passing value to parser */
extern int lineNum;
extern char* yytext; /* for storing variable names */
  
%}


%%

[ \t]+ {}

[\n] { 
	lineNum++; 
	return '\n';
}

"(" {
	#ifdef DEBUG
	printf("token '(' at line %d\n", lineNum);
	#endif
	return '(';
	}

")" {
	#ifdef DEBUG
	printf("token ')' at line %d\n", lineNum);
	#endif
	return ')';
	}

"+" {
	#ifdef DEBUG
	printf("token '+' at line %d\n", lineNum);
	#endif
	return '+';
	}

"*" {
	#ifdef DEBUG
	printf("token '*' at line %d\n", lineNum);
	#endif
	return '*';
	}

"-" {
	#ifdef DEBUG
	printf("token '-' at line %d\n", lineNum);
	#endif
	return '-';
	}

"/" {
	#ifdef DEBUG
	printf("token '/' at line %d\n", lineNum);
	#endif
	return '/';
	}

"=" {
	#ifdef DEBUG
	printf("token '=' at line %d\n", lineNum);
	#endif
	return '=';
	}

[0-9]+ {
	#ifdef DEBUG
	printf("token %s at line %d\n", yytext, lineNum);
	#endif
	yylval.d = atoi(yytext);
	return NUM;
	}

"!" {
	#ifdef DEBUG
	printf("token %s at line %d\n", yytext, lineNum);
	#endif
	return '!';
	}

"**" {
	#ifdef DEBUG
	printf("token %s at line %d\n", yytext, lineNum);
	#endif
	return EXP;
	}

"?" {
	#ifdef DEBUG
	printf("token %s at line %d\n", yytext, lineNum);
	#endif
	return '?';
	}	

[a-zA-Z0-9]* {
	#ifdef DEBUG
	printf("token %s at line %d\n", yytext, lineNum);
	#endif
	sscanf(yytext, "%s", yylval.name);
	return VAR;
	}	

%%
int yywrap() { /* need this to avoid link problem */
	return 1;
}