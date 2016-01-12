#include "malloc.c"
#include <string.h>
#include <stdio.h>
int main(void){
	void* A = malloc(1);	
	void* B = malloc(1);
	void* C = malloc(1);
	void* D = malloc(1);
	
	free(A);
	free(B);
	free(C);
	free(D);
	
	A = malloc(1);
	B = realloc(A,10);
	free(A);
	free(B);

return 0;
}
