//#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <stddef.h>
#include <stdlib.h>

//#include "shm_header.h"
#include "table.h"
//using namespace std;

head_t * t_head = NULL;
void * shm_head = NULL;
int table_size;
void * process_input(void * args) {
	req_t *req = (req_t *)args;
	e_op op = req -> op;
	switch(op) {
		case INS: hash_insert(shm_head, t_head, req, table_size);
			break;
		case RD: hash_read(shm_head, t_head, req, table_size);
			break;
		case DEL: hash_delete(shm_head, t_head, req, table_size);
			break;
	}
	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char ** argv)
{
	int shm_fd;
	pthread_t *tid;
        //ftruncate(shm_fd, SHMSIZE);
	//t_head = init_table(table_size);
	req_t req;
	void * arg = (void *)&req;
	printf("Hash table created and initiatialized\n");
	shm_fd = shm_open(SHMNAME, O_RDWR | O_CREAT, 0660);   // create shm queue
	table_size = atoi(argv[1]);
        t_head = init_table(table_size);
	if(table_size <= 0) {
		printf("Table size must be a natural number\n");
		exit(0);
	}
	if (shm_fd < 0) {
                printf("Failed to create shm\n");
                exit(0);
        }
	ftruncate (shm_fd, SHMSIZE);
	shm_head = (void *) mmap (NULL, SHMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	init_queue(shm_head);
	req_t * req_ptr = (req_t *)((wait_t *)((global_t *) shm_head + 1) + MAX_CLIENTS);
	printf("Queue initialized\n");
	tid = (pthread_t *) malloc (sizeof(pthread_t) * MAX_CLIENTS);
	while(1) {
		if (dequeue(shm_head, req_ptr, &req)) {
			pthread_create(tid+(req.id), NULL, process_input, (void *)&req);
		}
		
	}
	
	return 0;
}

