#include <stdio.h>
#include <stdlib.h>

#include "treeStructure.c"

int main(int argc, char* argv[]) {
	treeStruct* treeAST = treeGeneration(argv[1], argv[2]);
	//printf(psr->adaptor->makeDot(psr->adaptor, tree)->chars);
	//printf(treeAST->root);
	//if (treeAST != NULL) {
	//	freeEr();
	//}
	//fclose(dotPath);
	freeErrors();
	//printf("Bu ispugalsa??\n");
	return 0;
}