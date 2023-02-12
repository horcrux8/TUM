//#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "shm_header.h"


//using namespace std;

int main(int argc, char ** argv)
{
#if 0
	// ftok to generate unique key
	key_t key = ftok("hashoper_shm",65);

	// shmget returns an identifier in shmid
	int shmid = shmget(key,1024,0666|IPC_CREAT);

	// shmat to attach to shared memory
	char *str = (char*) shmat(shmid,(void*)0,0);

	printf("Data read from memory: %s\n",str);
	
	//detach from shared memory
	shmdt(str);
	
	// destroy the shared memory
	shmctl(shmid,IPC_RMID,NULL);
#endif
	int myid = 0, op;
	wait_t * wait_ptr = NULL;
	req_t * req_ptr = NULL;
	req_t input;
	int shm_fd = shm_open(SHMNAME, O_RDWR, 0660);	//open shm queue
	if (shm_fd < 0) { 
		printf("Server is not up\n");
		exit(0);
	}
	void *shm_head = (void *) mmap (NULL, SHMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	wait_ptr = (wait_t *)((global_t *)shm_head + 1);
	while(myid < MAX_CLIENTS) {
		if(wait_ptr -> occupied)
			wait_ptr++;
		else {
			pthread_mutex_lock(& (wait_ptr -> lock));
			if(wait_ptr -> occupied == 0) {
				wait_ptr -> occupied = 1;
				wait_ptr -> reply = 1;
				pthread_mutex_unlock (& (wait_ptr -> lock));
				break;
			}
			else {
				pthread_mutex_unlock (& (wait_ptr -> lock));
				wait_ptr++;
			}
		}
		myid++;
	
	} 
	if(myid==MAX_CLIENTS) {
		printf("server is overloaded. Try again later.");
		exit(0);
	}
	req_ptr = (req_t *)((wait_t *)((global_t *)shm_head + 1) + MAX_CLIENTS);
	input.id = myid;
	printf("Enter command : 1:Insert, 2:Read, 3:Delete 4:To Quit, followed by key. \nFor example: 2 56\n");
	while(1) {
		printf("Input : ");
		scanf("%d", &op);
		if(op == 4) {
			release_id(wait_ptr);
			break;
		}
		scanf("%d",  &input.key);
		if(!op || (op > 3)) {
			printf("Invalid operation %d\n", op);
			printf("Enter command : 1:Insert, 2:Read, 3:Delete followed by key. \nFor example: 2 56\n");
			continue;
		}
		else {
			input.op = op;
			enqueue((global_t *)shm_head, wait_ptr, req_ptr, input);
			printf("Done\n");
		}
	}		
	close(shm_fd);
	shm_unlink(SHMNAME);
	//printf("%ld\t%ld\t%ld\t%ld\t%ld\n", sizeof(temp_glob), sizeof(temp_req), sizeof(temp_wait), sizeof(temp), sizeof(temp_wait.lock));
	return 0;
}

