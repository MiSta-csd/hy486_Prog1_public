/*
    CS486 - Programming Assignment1, 2022
    Author: Minos Stavrakakis - csd4120

*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "DLList.h"
#include "HTable.h"
#include "ULFStack.h"

#define TRUE 1
#define FALSE 0

typedef struct thread_args {
	int given_id; // thread id
	int total_threads;
} thread_args;

pthread_barrier_t thread_barrier;

int mainRetVal=0;
int t_status[9] = {0}; // Made this for testing (for 9 threads) thread status
pthread_t *t_array; // Self exmplanatory for thread inter-comm (mainly kill)

int PERISH = FALSE; // Global var that Thread[0] (& potentially all threads)
// uses to learn if it is to die

// For testing purposes. Thread status
void PrintWork() {

	char *prntOutput[] = { "Not Started", "Working", "DONE!"};

	fprintf(stdout,"~~~~~~~~~~~~~~~~~~~\n");
	for (int i=0; i<9; i++) {
		fprintf(stdout, "Thread %d is: %s\n", i, prntOutput[t_status[i]]);
	}
	fprintf(stdout,"~~~~~~~~~~~~~~~~~~~\n\n");
}

// 
void killemAll(pthread_t *t_arr, int t_id, int N_threads) {
	int i;
	for (i=0; i<N_threads; i++) {
		
		if(i == t_id)
			continue;
		else
			pthread_cancel(t_arr[i]);
	}
	//pthread_exit(NULL);
}

void *
thread_main(void *myargs) {


	thread_args *t_arg = (thread_args*) myargs;
	// Transfer args to local vars for eou
	int N_threads = t_arg->total_threads;
	int t_id = t_arg->given_id;
	// Update gloabal t_array
	t_array[t_id] = pthread_self();

	int cnt, productID, dll_size, dll_keySum;
	int N_sqr = N_threads * N_threads;
	int KeySumMax = (N_sqr * (N_sqr-1))/2;
	int M_tables = N_threads / 3;
	int ht_elem_count, ht_keysum=0;
	int stck_sz,i;

	// ~~~ Producer phase ~~~
	// calculate productID & insert to the list
	for (cnt = 0; cnt < N_threads; cnt++) {
		
		productID = (N_threads * cnt)  + t_id;
		DLLInsert(productID);
	}
	// make sure all insertions are completed
	pthread_barrier_wait(&thread_barrier);
	
	// thread[0] performs scans
	if(t_id == 0) {
		
		dll_size = DLLSize();
		dll_keySum = DLLKeySum();
		
		fprintf(stdout, 
		"List size check (expected: %d , found: %d)\n", 
		N_sqr, dll_size);

		if((dll_size != N_sqr)) {
			fprintf(stderr,
			"ERROR: List Size Check Failed.\n");
			mainRetVal = -1;
			killemAll(t_array, t_id, N_threads);
			//pthread_exit(NULL);
			PERISH = TRUE;
		}

		fprintf(stdout, 
		"List keysum check (expected: %d , found: %d)\n", 
		KeySumMax, dll_keySum);

		if((dll_keySum != KeySumMax)) {
			fprintf(stderr,
			"ERROR: List Keysum Check Failed.\n");
			mainRetVal = -1;
			killemAll(t_array, t_id, N_threads);
			PERISH = TRUE;
		}

	}

	// Waiting thread[0] to finish checks
	pthread_barrier_wait(&thread_barrier);
	
	if(PERISH) pthread_exit(NULL);

	// ~~~ Seller phase ~~~
	for (cnt = 0; cnt < N_threads; cnt++) {
		
		productID = (N_threads * t_id)  + cnt;
		// If pID found & deleted, insert to appr hashTable
		if(DLLDelete(productID)) {
			HTInsert(productID, 
				(t_id + cnt) % M_tables);
		}
		else {
			fprintf(stderr,
			"ERROR: productID to-be deleted not found in list\n");
			mainRetVal = -1;
			killemAll(t_array, t_id, N_threads);
			PERISH = TRUE;
			break;
		}
		
	}
		
	// make sure all insertions are completed
	pthread_barrier_wait(&thread_barrier);

	if(PERISH) pthread_exit(NULL);
	// 
/* 	if(t_id == 0)  {
			
		// HTPrintAllTAbles();
		// PrintWork();
		printf("List size = %d\n", DLLSize());
	}

	pthread_barrier_wait(&thread_barrier); */
	
	// thread[0] performs checks
	if(t_id == 0) {

		for (cnt=0; cnt<M_tables; cnt++) {
			
			ht_elem_count = HTGetTableStats(NO_ELEMS, cnt);
			// ht_elem_count = HTGetNoElems(cnt); // valid function too
			ht_keysum += HTGetTableStats(KEY_SUM, cnt);

			fprintf(stdout, 
			"HT[%d] size check (expected: %d , found: %d)\n", 
			cnt, 3*N_threads, ht_elem_count);

			if((ht_elem_count != 3*N_threads)) {
				fprintf(stderr,
				"ERROR: HT[%d] size check failed.\n", cnt);
				mainRetVal = -1;
				killemAll(t_array, t_id, N_threads);
				PERISH = TRUE;
				break;
			}
		}

		fprintf(stdout, 
		"HT keysum check (expected: %d , found: %d)\n", 
		KeySumMax, ht_keysum);

		if((ht_keysum != KeySumMax)) {
			fprintf(stderr,
			"ERROR: HT keysum check failed.\n");
			mainRetVal = -1;
			killemAll(t_array, t_id, N_threads);
			PERISH = TRUE;
		}
	}

	// Waiting thread[0] to finish checks
	pthread_barrier_wait(&thread_barrier);

	if(PERISH) pthread_exit(NULL);


	// ~~~ Admin phase ~~~
	i = 0;
	while (i<M_tables) { // Try until you successfully del/ins M pIDs.

		productID = (N_threads * (rand() % N_threads))  + (rand() % N_threads);

		if (HTDelete(productID, 
		(t_id + i) % M_tables)) {
			ULFS_push(productID);
			++i;
		}
	}

	// make sure all insertions are completed
	pthread_barrier_wait(&thread_barrier);

	// thread[0] performs scans
	if(t_id == 0) {

		for (cnt=0; cnt<M_tables; cnt++) {
			
			ht_elem_count = HTGetTableStats(NO_ELEMS, cnt);
			stck_sz = ULFS_getSize2();

			fprintf(stdout, 
			"HT[%d] size check (expected: %d , found: %d)\n", 
			cnt, 2*N_threads, ht_elem_count);

			if((ht_elem_count != 2*N_threads)) {
				fprintf(stderr,
				"ERROR: HT[%d] size check failed.\n", cnt);
				mainRetVal = -1;
				killemAll(t_array, t_id, N_threads);
				PERISH = TRUE;
				break;
			}

		}

		fprintf(stdout, 
		"Stack size check (expected: %d , found: %d)\n", 
		N_sqr/3, stck_sz);

		if((stck_sz != N_sqr/3)) {
			fprintf(stderr,
			"ERROR: Stack size check failed.\n");
			mainRetVal = -1;
			killemAll(t_array, t_id, N_threads);
			PERISH = TRUE;
		}
	}

	// Waiting thread[0] to finish checks
	pthread_barrier_wait(&thread_barrier);

	if(PERISH) pthread_exit(NULL);

	// ~~~ Re-Insert to List phase ~~~
	while(1) {
		
		productID = ULFS_pop();
		
		if(productID != -1) {
			DLLInsert(productID);
		}
		else {
			break;
		}
	}


	// make sure all insertions are completed
	pthread_barrier_wait(&thread_barrier);

	// Ckecks
	//	..... list sz exp n^2 / 3
	// List size check (expected: N2/3, found: Y)
	if(t_id == 0) {

		dll_size = DLLSize();
		// stck_sz = ULFS_getSize2();

		// printf("Stack size = %d\n", stck_sz);

		fprintf(stdout, 
		"List size check (expected: %d , found: %d)\n", 
		N_sqr/3, dll_size);
	
		if((dll_size != N_sqr/3)) {
			fprintf(stderr,
			"ERROR: List Size Check Failed.\n");
			mainRetVal = -1;
			killemAll(t_array, t_id, N_threads);
			PERISH = TRUE;
		}
	
	}
	
	// Waiting thread[0] to finish checks
	pthread_barrier_wait(&thread_barrier);

	if(PERISH) pthread_exit(NULL);

	return NULL;
}


int main(int argc, char const *argv[])
{
	int N,i;
	
	// Foolproof user input :D
	if((argc > 1)&&((((N = atoi(argv[1]))%3) == 0)));
	else N = 3;
	
	pthread_t my_threads[N];
	thread_args arg[N];

	//Initialize Structures
	mainRetVal=0;
	DLLInit();
	HTInit(N);
	ULFS_init();
	assert((t_array = malloc(sizeof(pthread_t) * N)));

	pthread_barrier_init (&thread_barrier,
	 					NULL, N);

	for (i=0; i<N; i++) {
		arg[i].given_id = i;
		arg[i].total_threads = N;

		pthread_create (&(my_threads[i]), NULL, 
		thread_main, (void*)(&arg[i]));
	}

	for (i=0; i<N; i++) {
		pthread_join((my_threads[i]), NULL);
	}
	
	// printf("Value of N = %d \n", N);

	// Clear Structures
	DLLDestroy();
	HTDestroy();
	ULFS_destroy();
	free(t_array);
	fprintf(stdout, "Program exited with code: %d.\n", mainRetVal);
	
	return mainRetVal;
}
