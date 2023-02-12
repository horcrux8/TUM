#include <pthread.h>
#include <semaphore.h>
#include "shm_header.h"

typedef struct ele_s{
	int key;
	struct ele_s * next;
}ele_t;

typedef struct table_s
{
	int size;
	//pthread_mutex_t lock;
	sem_t reader;
	sem_t writer;
	ele_t *lst;
	
}table_t;

typedef struct head_s {
        int size;
	table_t *bin;
}head_t;

head_t * init_table(int table_size);
void hash_read(global_t* shm_head, head_t * t_head, req_t* req, int table_size);
void hash_insert(global_t* shm_head, head_t * t_head, req_t* req, int table_size);
void hash_delete(global_t* shm_head, head_t * t_head, req_t* req, int table_size);
