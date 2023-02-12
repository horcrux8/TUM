//#include <types.h>
#include <stdint.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdio.h>

#include "shm_header.h"


void init_queue(void * shm_head) {
	int i=0;
	sem_init (&(((global_t *)shm_head) -> queue_mut), 1, 1);	
	((global_t *)shm_head) -> rear = -1;
	((global_t *)shm_head) -> front = -1;
	wait_t * wait_head = (wait_t *)shm_head + 1;
	for(i=0; i < MAX_CLIENTS; i++) {
		(wait_head + i) -> occupied = 0;
		pthread_mutex_init( &((wait_head + i) -> lock), NULL); 
	}
}

void enqueue(global_t * shm_head, wait_t * wait_ptr, req_t * req_ptr, req_t input) {
	sem_wait(&(shm_head -> queue_mut));
	if(((MAX_CLIENTS-1) == shm_head -> rear) || (shm_head -> rear == shm_head -> front -1)) {
		printf("Queue full, packet dropped\n");
		return;
	}
	else if(shm_head -> front == -1) { // first element
		shm_head ->rear = 0;
		shm_head ->front = 0;
		*req_ptr = input; 
	
	}
	else if(shm_head -> rear == MAX_CLIENTS - 1) {
		shm_head -> rear = 0;
		*req_ptr = input;
	}
	else {
		shm_head -> rear++;
		*(req_ptr + shm_head -> rear) = input;
	}
	
	wait_ptr -> reply = 0;
	sem_post(&(shm_head -> queue_mut));
	while(wait_ptr -> reply == 0) {
        }
	if(input.op == 2) {
		if(wait_ptr -> reply == 1) 
			printf("Key %d found in hashtable\n", input.key);
		else
			printf("Key %d not found in hashtable\n", input.key);
	}
}

void release_id(wait_t * wait_ptr) {
	pthread_mutex_lock(& (wait_ptr -> lock));
	wait_ptr -> occupied = 0;
	wait_ptr -> reply = 1;
	pthread_mutex_unlock (& (wait_ptr -> lock));
}

int dequeue(global_t* shm_head, req_t * req_ptr, req_t * req_data) {
	if(shm_head -> front == -1) 	//empty queue
		return 0;
	sem_wait(&(shm_head -> queue_mut));
	*req_data = *(req_ptr + shm_head -> front);
	if(shm_head -> front == shm_head -> rear) {
		shm_head -> front = -1;
		shm_head -> rear = -1;
	}
	else if(shm_head -> front == MAX_CLIENTS -1) {
		shm_head -> front = 0;
	}
	else
		shm_head -> front ++;
	sem_post(&(shm_head -> queue_mut));
	return 1;
} 
