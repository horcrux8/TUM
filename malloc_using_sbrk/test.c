#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "cust_malloc.h"

#define MALLOC(sz) dmalloc(sz)
#define FREE(p)    dfree(p)



int main(int argc, char *argv[])
{
	int * temp =(int *) malloc(sizeof(int)*10);
	int i=0;
	for( i=0; i<10; i++) {
		*(temp+i) = i*2;
	}
	for( i=0; i<10; i++) {
                printf("%d\n",*(temp+i));
        }
	free(temp);
 	return 0;
}
