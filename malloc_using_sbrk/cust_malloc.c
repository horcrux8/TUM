#include <stdio.h> 
#include <unistd.h>
#include <pthread.h>
#include "cust_malloc.h"

// meta data structure size
#define META_SZ sizeof(struct blk_s)

// lock for sbrk
pthread_mutex_t sbrk_lock = PTHREAD_MUTEX_INITIALIZER;
// lock for linked list of blocks
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// head to the linked list of blocks both freed or allocated
void * head;


// meta data to store block inormation
typedef struct blk_s {
	size_t size;      //  = block size - META_SZ
	struct blk_s *next;
	struct blk_s *prev;
	int free;
} blk_t;

// first fit algorithm to search free block if available in the linked list
blk_t * find_blk(blk_t ** last, size_t size_req) {
	//blk_t *new=last;
	blk_t *fit = head;
	while(fit != NULL) {
		if(fit->free && (fit->size >= META_SZ + size_req)) {
			return fit;
		}	
		//if a block is not found, last points to the last node of the list, 
		//saves us the time to again parse the linked list to connect the new allocated node
		*last = fit;
		fit = fit -> next;
	}
	//if(first == NULL)
	return NULL;
}

// did not find a block in the linked list, allocate new block using using sbrk()
blk_t * alloc_blk(blk_t * last, size_t size_req) {
	blk_t * nblk;
	// hold the lock
	pthread_mutex_lock(&sbrk_lock);
	nblk = sbrk(0);
	void *prev_pbreak = sbrk(size_req + META_SZ);
	// release the lock 
	pthread_mutex_unlock(&sbrk_lock);
	if(prev_pbreak == (void *) -1) {	//sbrk failed
		return NULL;
	}
	if(last) {
		last->next = nblk;
	}
	nblk -> size = size_req;
	nblk -> free = 0;
	nblk -> next = NULL;
	nblk -> prev = last;
	return nblk;
}

// if the block size is greater than requested, split it
void split(blk_t * blk, size_t size_req) {
	if(blk) {
		blk_t * extra = (void *)(blk + META_SZ + size_req);
		extra -> next = blk -> next;
		extra -> prev = blk;
		extra -> size = (blk -> size) - (META_SZ + size_req);
		extra -> free = 1;
		if(extra -> next) {
			(extra->next) -> prev = extra;
		}
		blk -> size = size_req;
		blk -> next = extra;
		blk -> free = 0;

	}
}

//customized malloc
void * cust_malloc(size_t size_req) {
	if(size_req <= 0) 
		return NULL;
	else {	// valid size request
		pthread_mutex_lock(&lock);
		blk_t * blk;
		blk_t * last;
		if(head == NULL) { // first malloc, list is empty
			blk = alloc_blk(NULL, size_req);
			if(!blk) {	// couldn't allocate
				pthread_mutex_unlock(&lock);
				return NULL;
			}
			else { //block allocated
				head = blk;
				pthread_mutex_unlock(&lock);
				//printf("allocated address: %lx\t%lx\tsize:%ld\n", blk, blk+1, size_req+META_SZ);
				return (blk + 1); // return address of first byte after meta data
			}
		}
		else { // list is not empty
			//last = head;
			blk = find_blk(&last, size_req);	
			if(blk) { // fit block found
				if(blk -> size >= size_req + META_SZ)
					split(blk, size_req);
				pthread_mutex_unlock(&lock);
				//printf("allocated address: %lx\tsize:%ld\n", blk, size_req+META_SZ);
                                return (blk + 1); // return address of first byte after meta data
			} 
			else {	// fit block not found in the list, allocate a new one
				blk = alloc_blk(last, size_req);
                        	if(!blk) {      // allocation failed
                                	pthread_mutex_unlock(&lock);
                                	return NULL;
                        	}
                        	else { //block allocated
                                	pthread_mutex_unlock(&lock);
                                	return (blk + 1); // return address of first byte after meta data
                        	}
			}
		}
	}
}

// addr points to block_address+meta_data
blk_t * actual_addr(blk_t * addr) {
	return ((blk_t *)addr - 1);
}

// merging of free blocks
void merge(blk_t * blk) {
	if(blk -> next) { // if next block is free
		if(blk -> next -> free) {
			blk -> size += META_SZ + blk -> next -> size;
			blk -> next = blk -> next -> next;
			if (blk -> next)
				blk -> next -> prev = blk ;
		}
	}  
	if(blk -> prev) { // if previous block is free
		if(blk -> prev -> free) {
			blk = blk -> prev;
                        blk -> size += META_SZ + blk -> next -> size;
			blk -> next = blk -> next -> next;
			if (blk -> next)
                                blk -> next -> prev = blk ;
		}
		
	}
}

//customized free
void cust_free(void *addr) {
	pthread_mutex_lock(&lock);
	if(!addr) { //invalid address
		pthread_mutex_unlock(&lock);
		return;
	}
	else {
		blk_t * blk_start = actual_addr(addr);
		if(blk_start) {
			blk_start -> free = 1;
			merge(blk_start);
			pthread_mutex_unlock(&lock);
			return;
		}
		else {
			pthread_mutex_unlock(&lock);
                        return;
		}
	}
}

void * dmalloc(size_t size) {
	void *p = cust_malloc(size);
	return p;
}

void dfree(void *addr) {
	cust_free(addr);
}	
