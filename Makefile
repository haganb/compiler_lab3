infix: infix.tab.o lex.yy.o node.o stack.o blocks.o latency.o
	gcc -o infix lex.yy.o infix.tab.o node.o stack.o blocks.o latency.o

lex.yy.o: infix.l
	flex infix.l; gcc -c lex.yy.c

infix.tab.o: infix.y
	bison -d infix.y; gcc -c infix.tab.c

stack.o: stack.c 
	gcc -c stack.c

node.o: node.c
	gcc -c node.c

latency.o: latency.c 
	gcc -c latency.c

blocks.o: blocks.c 
	gcc -c blocks.c

clean:
	rm -f p2 infix.output *.o infix.tab.c lex.yy.c
