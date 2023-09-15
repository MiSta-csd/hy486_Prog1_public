/*
Implementation based on CS486 - section5 p19

				Linked Lists
	
• 	We implement a set as a linked list of nodes.

• 	The list contains regular nodes and two sentinel
	nodes, called head and tail, that point to the first 
	and last element of the list, resultpectively.

• 	Sentinel nodes are never inserted or deleted; the key 
	of head is MININT and the key of tail is MAXINT.

• 	The list is sorted in key order.

• 	Each process p uses two pointers, curr and prev to
	traverse the list; currr points to the currrent node 
	accessed in the list and prev to its predious node.

•	Fine Grained Sunchronaization is implemented by 
	hand-over-hand locking.

*/

#include "DLList.h"
#include <limits.h>
#include <stdlib.h>
#include <assert.h>


#define TRUE 1
#define FALSE 0


typedef struct DLLNode {
	int key; // productID
	pthread_mutex_t lock;
	struct DLLNode *next;
	struct DLLNode *prev; 
} DLLNode;

// initially, head points to a dummy node with key
// equal to MININT and tail points to a dummy
// node with key equal to MAXINT
DLLNode *Head, *Tail;


void DLLInit(void) {
    
    assert((Head = ((DLLNode*) malloc(sizeof(DLLNode)))));
    assert((Tail = ((DLLNode*) malloc(sizeof(DLLNode)))));
    
    Head->key = INT_MIN;
    pthread_mutex_init(&Head->lock, NULL);
    Head->next = Tail;
    Head->prev = NULL;
    

    Tail->key = INT_MAX;
    pthread_mutex_init(&Tail->lock, NULL);
    Tail->prev = Head;
    Tail->next = NULL;

}


int DLLSearch(int key) {

	DLLNode *curr, *prev;
	int result;

	// try until node locks
    while(pthread_mutex_lock(&Head->lock));
    prev = Head;
	curr = prev->next;
	while(pthread_mutex_lock(&curr->lock));
	
	while(curr->key < key && (curr != Tail)) {
		pthread_mutex_unlock(&prev->lock);
		prev = curr;
		curr = curr->next;
		while(pthread_mutex_lock(&curr->lock));
	}

	if(key == curr->key) result = TRUE;
	else result = FALSE;
	pthread_mutex_unlock(&prev->lock);
	pthread_mutex_unlock(&curr->lock);
	return result;
}

int DLLInsert(int key) {
	DLLNode *prev, *curr, *new_node;
	int result;

	// try until node locks
    while(pthread_mutex_lock(&Head->lock));
    prev = Head;
	curr = prev->next;
	while(pthread_mutex_lock(&curr->lock));
	
	while(curr->key < key && (curr != Tail)) {
		pthread_mutex_unlock(&prev->lock);
		prev = curr;
		curr = curr->next;
		while(pthread_mutex_lock(&curr->lock));
	}

	if(key == curr->key) result = FALSE;
	else {
		assert((new_node = ((DLLNode*) malloc(sizeof(DLLNode)))));
		pthread_mutex_init(&new_node->lock, NULL);
		new_node->next = curr;
		new_node->prev = prev;
		curr->prev = new_node;
		prev->next = new_node;
		new_node->key = key;
		result = TRUE;
	}

	pthread_mutex_unlock(&prev->lock);
	pthread_mutex_unlock(&curr->lock);
	return result;
}

int DLLDelete(int key) {
	DLLNode *prev, *curr, *next;
	int result;

	// try until node locks
    while(pthread_mutex_lock(&Head->lock));
    prev = Head;
	curr = prev->next;
	while(pthread_mutex_lock(&curr->lock));
	
	while(curr->key < key && (curr != Tail)) {
		pthread_mutex_unlock(&prev->lock);
		prev = curr;
		curr = curr->next;
		while(pthread_mutex_lock(&curr->lock));
	}
	
	if(key == curr->key) {
		next = curr->next;
		while(pthread_mutex_lock(&next->lock));

		next->prev = prev;
		prev->next = next;

		pthread_mutex_unlock(&curr->lock);
		// WARNING !!! Use with caution.
				free(curr);
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		pthread_mutex_unlock(&prev->lock);
		pthread_mutex_unlock(&next->lock);
		result = TRUE;
	}
	else {
		pthread_mutex_unlock(&prev->lock);
		pthread_mutex_unlock(&curr->lock);
		result = FALSE;
	}

	return result;
}

void DLLDestroy(void) {
	DLLNode *curr, *prev;

	prev = Head;
	curr = prev->next;

	while (curr->next != NULL) {
		free(prev);
		prev = curr;
		curr = curr->next;
	}

	free(curr);
}

int DLLSize(void) {
	DLLNode *curr;
	int counter=0;

	curr = Head->next;

	while (curr != Tail) {
		++counter;
		curr = curr->next;
	}

	return counter;
}

int DLLKeySum(void) {
	DLLNode *curr;
	int counter=0;

	curr = Head->next;

	while (curr != Tail) {
		counter += curr->key;
		curr = curr->next;
	}

	return counter;
}