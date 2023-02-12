//#include <types.h>
#include <stdint.h>
#include <semaphore.h>
#define SHMNAME "hashtable_server_queue"
#define MAX_CLIENTS 10
#define SHMSIZE MAX_CLIENTS*(sizeof(wait_t)+sizeof(req_t))+sizeof(global_t)

typedef enum {
	INS = 1,
	RD,
	DEL
} e_op;

typedef enum {
	W,
	T,
	F,
	I
} e_rep;

typedef struct wait_s {
	int occupied;
	e_rep reply;
	pthread_mutex_t lock;
} wait_t;

typedef struct req_s {
        int id;
        e_op op;
        int key;
        int reply;
} req_t;

typedef struct global_s {
	//req_t *head;
	//req_t *rear;	
	//req_t *front;
	int rear;
	int front;
	sem_t queue_mut;

} global_t;

void init_queue(void * shm_head); 

void enqueue(global_t * shm_head, wait_t * wait_ptr, req_t * req_ptr, req_t input); 

void release_id(wait_t * wait_ptr); 

int dequeue(global_t* shm_head, req_t * req_ptr, req_t * req_data);
