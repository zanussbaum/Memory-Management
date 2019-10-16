#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>

#include "kmalloc.h"
#include "ll_double.h"

#define MIN(X, Y) (X < Y ? X : Y);

#define NUM_THREADS 8

pthread_mutex_t lock;

void *threadFunction(void * data) {
	// Allocate at most 512K in total
	//   8 threads should *not* be able to allocate 512KB within 4MB of memory because of the bookkeeping information
	//   kept by the internal implementation of kmalloc()
	int remaining_bytes = 524288;

	// Keeps track of the allocated chunks
	struct list allocated_chunks;

	printf("locked\n");
	pthread_mutex_unlock(&lock);
	ll_init(&allocated_chunks);
	pthread_mutex_unlock(&lock);
	printf("unlocked\n");

	// Allocation counter
	int counter = 0;

	while(remaining_bytes > 0) {
		// For every 10 kmalloc() calls, do a kfree() call
		pthread_mutex_lock(&lock);
		if(counter % 10) {
			struct node *first = ll_remove_head(&allocated_chunks);

			if(first) {
				printf("freeing\n");
				kfree(first->data);
			}
		}
		else {
			// Allocate between 1 and 4K bytes, up to the maximum of remaining bytes
			int chunk_size = MIN(remaining_bytes, (rand() % 4096) + 1);
			
			printf("malloc\n");
			char *memory = kmalloc(chunk_size);
			printf("memory address: %s\n", memory);

			if(!memory) {
				fprintf(stderr, "Memory is full (this is OK with limited memory), exiting");
				fflush(stdout);
				break;
			}

			remaining_bytes -= chunk_size;
			ll_insert_head(&allocated_chunks, memory);
		}
		pthread_mutex_unlock(&lock);
	}

	// Frees all data previously allocated
	struct node *first;

	pthread_mutex_lock(&lock);
	while((first = ll_remove_head(&allocated_chunks))) {
		printf("free from list\n");
		kfree(first->data);
	}
	pthread_mutex_unlock(&lock);

	return NULL;
}

int main(void) {
	// Feed the random number generator
	srand(time(NULL));

	pthread_t threadNumber[NUM_THREADS];
	int threadData[NUM_THREADS];

	for(int i = 0; i < NUM_THREADS; i++) {
		threadData[i] = i;
	}

	pthread_attr_t config;

	pthread_attr_init(&config);
	pthread_attr_setdetachstate(&config, PTHREAD_CREATE_JOINABLE);

	pthread_mutex_init(&lock,NULL);

	// Launches all threads -- they are automatically started
	for(int i = 0; i < NUM_THREADS; i++) {
		if(pthread_create(&threadNumber[i], &config, threadFunction, (void *) &threadData[i]) != 0) {
			fprintf(stderr, "Error creating thread\n");

			return EXIT_FAILURE;
		}
	}

	// Blocks main thread until all others return

	for(int i = 0; i < NUM_THREADS; i++) {
		if(pthread_join(threadNumber[i], NULL) != 0) {
			fprintf(stderr, "Error joning threads\n");

			return EXIT_FAILURE;
		}
	}

	printf("All threads finished without crashing. That doesn't mean the program is right. Please check with debug.\n");
	return EXIT_SUCCESS;
}
