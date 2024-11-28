
build:
	gcc -g -O0 -o ./app ./main.c antlr/labgrammLexer.c antlr/labgrammParser.c ./treeStructure.c -lantlr3c  

clean:
	rm -f ./app

rebuild: clean build