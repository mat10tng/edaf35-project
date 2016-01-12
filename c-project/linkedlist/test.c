#include "malloc.c"
#include <string.h>
#include <stdio.h>
int main(void){
	void* A = malloc(1);	
	void* B = malloc(1);
	void* C = calloc(1,2);
	void* D = realloc(A,2);

	free(A);
	printf("---------\n");
	free(B);
	free(C);
	free(D);
return 0;
}
