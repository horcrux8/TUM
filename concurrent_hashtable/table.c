#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "table.h"
int readers = 0;

head_t * init_table(int table_size) {
	int i = 0;
	head_t * ntable = (head_t *) malloc (sizeof(head_t));
	
	ntable -> size = table_size;
	ntable -> bin = (table_t *) malloc (sizeof(table_t) * table_size);
	for(i=0; i<table_size; i++) {
		(ntable->bin + i) -> lst = NULL;
		//pthread_mutex_init
		sem_init(& ((ntable->bin + i) -> reader), 0, 1);
		sem_init(& ((ntable->bin + i) -> writer), 0, 1);
	}
	return ntable;
	
}

int find_bin(int key, int table_size){
	if(key <= 0){
		printf("invalid key value\n");
		return -1;
	}
	else{
		int hash = key % table_size;
		return hash;
	}
}

void hash_read(global_t* shm_head, head_t * t_head, req_t* req, int table_size) {
	int bin_id = -1;
	wait_t * wait_ptr = ((wait_t *)(shm_head + 1) + req -> id);
        if(req -> key <= 0) {
                //printf("Invalid key")
                pthread_mutex_lock(& (wait_ptr -> lock));
                wait_ptr -> reply = I;
                pthread_mutex_unlock (& (wait_ptr -> lock));

        }
        else {
	
                bin_id = find_bin(req -> key, table_size);
                table_t *bin_ptr = t_head -> bin + bin_id;
                //pthread_mutex_lock(&(bin_ptr -> lock));
		sem_wait(&(bin_ptr -> reader));
		readers++;
		if(readers==1)	//first reader
        		sem_wait(&(bin_ptr -> writer));
		sem_post(&(bin_ptr -> reader));
                if(bin_ptr -> lst == NULL) {    //bin empty
			//printf("bin empty\n");
			pthread_mutex_lock(& (wait_ptr -> lock));
                        wait_ptr -> reply = F;
                        pthread_mutex_unlock (& (wait_ptr -> lock));
			sem_wait(&(bin_ptr -> reader));
                        readers--;
                        if(readers==0)      //last reader
                        {
                        	sem_post(&(bin_ptr -> writer));
                        }
                        sem_post(&(bin_ptr -> reader));
		}
		else {
			ele_t *temp = bin_ptr -> lst;
			
                        while(temp != NULL) {
                                if(temp -> key == req -> key) {    //key already exists
					
					pthread_mutex_lock(& (wait_ptr -> lock));
                                        wait_ptr -> reply = T;
                                        pthread_mutex_unlock (& (wait_ptr -> lock));
					sem_wait(&(bin_ptr -> reader));
					readers--;
					if(readers==0)	//last reader
    					{
        					sem_post(&(bin_ptr -> writer));
    					}
					sem_post(&(bin_ptr -> reader));
                                        return;
                                }
                                temp = temp -> next;
                        }

		}
		pthread_mutex_lock(& (wait_ptr -> lock));
		wait_ptr -> reply = F;
		pthread_mutex_unlock (& (wait_ptr -> lock));
		sem_wait(&(bin_ptr -> reader));
                        readers--;
                        if(readers==0)      //last reader
                        {
                                sem_post(&(bin_ptr -> writer));
                        }
                        sem_post(&(bin_ptr -> reader));
	}

}

void hash_insert(global_t* shm_head, head_t * t_head, req_t* req, int table_size) {
	int bin_id = -1;
	wait_t * wait_ptr = ((wait_t *)(shm_head + 1) + req -> id);
	if(req -> key <= 0) {
		//printf("Invalid key")
		pthread_mutex_lock(& (wait_ptr -> lock));
        	wait_ptr -> reply = I;
        	pthread_mutex_unlock (& (wait_ptr -> lock));

	}
	//printf("Inserting\n");
	else {
		
		bin_id = find_bin(req -> key, table_size);
		table_t *bin_ptr = (t_head -> bin) + bin_id;
		//pthread_mutex_lock(&(bin_ptr -> lock));
		sem_wait(&(bin_ptr -> writer));
		if(bin_ptr -> lst == NULL) {	//bin empty
			ele_t * new_ele = (ele_t  *)malloc(sizeof (ele_t));
			new_ele -> key = req -> key;
			new_ele -> next = NULL;
			bin_ptr -> lst = new_ele;
			pthread_mutex_lock(& (wait_ptr -> lock));
			wait_ptr -> reply = T;
			pthread_mutex_unlock (& (wait_ptr -> lock));
			//pthread_mutex_unlock(&(bin_ptr -> lock));
			sem_post(&(bin_ptr -> writer));
		}
		else {
			ele_t *temp = bin_ptr -> lst;
			ele_t *prev = NULL;
			while(temp != NULL) {
				if(temp -> key == req -> key) {	//key already exists
					pthread_mutex_lock(& (wait_ptr -> lock));
					wait_ptr -> reply = T;
					pthread_mutex_unlock (& (wait_ptr -> lock)); 
					sem_post(&(bin_ptr -> writer));
					return;
				}
				prev = temp;
				temp = temp -> next;
			}
			ele_t * new_ele = (ele_t  *)malloc(sizeof (ele_t));
			new_ele -> key = req -> key;
			new_ele -> next = NULL;
			prev -> next = new_ele;
			pthread_mutex_lock(& (wait_ptr -> lock)); 
			wait_ptr -> reply = T;
			pthread_mutex_unlock (& (wait_ptr -> lock));
			//pthread_mutex_unlock(&(bin_ptr -> lock)); 
			sem_post(&(bin_ptr -> writer));
		}
		
	}
}

void hash_delete(global_t* shm_head, head_t * t_head, req_t* req, int table_size) {
	int bin_id = -1;
        wait_t * wait_ptr = ((wait_t *)(shm_head + 1) + req -> id);
        if(req -> key <= 0) {
                //printf("Invalid key")
                pthread_mutex_lock(& (wait_ptr -> lock));
                wait_ptr -> reply = I;
                pthread_mutex_unlock (& (wait_ptr -> lock));

        }
        else {
                bin_id = find_bin(req -> key, table_size);
                table_t *bin_ptr = t_head -> bin + bin_id;
                sem_wait(&(bin_ptr -> writer));
                if(bin_ptr -> lst == NULL) {    //bin empty
                        pthread_mutex_lock(& (wait_ptr -> lock));
                        wait_ptr -> reply = T;
                        pthread_mutex_unlock (& (wait_ptr -> lock));
                        sem_post(&(bin_ptr -> writer));
                }
                else {
                        ele_t *del = bin_ptr -> lst;
			ele_t * temp = NULL;
                        while(del != NULL) {
                                if(del -> key == req -> key) {    //key found
					if(temp == NULL)
						bin_ptr -> lst = NULL;
					else 
						temp -> next = del -> next;
					free(del);
                                        pthread_mutex_lock(& (wait_ptr -> lock));
                                        wait_ptr -> reply = T;
                                        pthread_mutex_unlock (& (wait_ptr -> lock));
                                        sem_post(&(bin_ptr -> writer));
                                        return;
                                }
                                temp = del;
				del = del -> next;
                        }
                        pthread_mutex_lock(& (wait_ptr -> lock));
                        wait_ptr -> reply = T;
                        pthread_mutex_unlock (& (wait_ptr -> lock));
                        sem_post(&(bin_ptr -> writer));

                }

        }
}
