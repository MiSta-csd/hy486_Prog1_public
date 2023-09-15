#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <limits.h>
#include "ULFStack.h"
//#include <stdatomic.h>

#define TRUE 1
#define FALSE 0

#define MIN_WAIT 500
#define MAX_WAIT 5000



typedef struct stackNode {
    int productID;
    struct stackNode *next;
} stackNode;

stackNode *EMPTY_STACK;

//the stack
typedef struct ULFStack
{
    int tag;
    unsigned size;
    stackNode *Top;
} ULFStack;

ULFStack *Stack;

void ULFS_init() {
    assert((Stack = malloc(sizeof(ULFStack))));
    // EMPTY_STACK is a dummy node
    assert((EMPTY_STACK = malloc(sizeof(stackNode))));

    EMPTY_STACK->next = NULL;
    EMPTY_STACK->productID = -1;

    Stack->Top = EMPTY_STACK;

    Stack->size = 0;
    Stack->tag = 0;
}

void ULFS_destroy() {
    stackNode *curr, *next;

    curr = Stack->Top;

    while (curr != NULL) {
        
        next = curr->next;
        free(curr);
        curr = next;
        
    }

    free(Stack);
}

void doNothing(){
    ;return;
}

void Backoff() {
    volatile int i;
    int wait;

    wait = MIN_WAIT + rand() % (MAX_WAIT+1 - MIN_WAIT);

    for (i=0; i<wait; i++) {
        doNothing();
    }
}

int TryPush(stackNode *newNode){
    stackNode *oldTop = Stack->Top;
    newNode->next = oldTop;

    if(__atomic_compare_exchange(&(Stack->Top), &oldTop, &newNode,
        TRUE, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)){
        __atomic_add_fetch(&(Stack->size), 1, __ATOMIC_SEQ_CST);
        return TRUE;
    }
    else return FALSE;
}

void ULFS_push(int key) {
    stackNode* newNode = malloc(sizeof(stackNode));
    newNode->productID = key;
    
    while(TRUE) {
        if (TryPush(newNode)) return;
        else Backoff();
    }
}

stackNode *TryPop() {
    stackNode *oldTop = Stack->Top;
    stackNode *newTop;

    if(oldTop == EMPTY_STACK)
        return EMPTY_STACK;

    newTop = oldTop->next;
    if(__atomic_compare_exchange(&(Stack->Top), &oldTop, &newTop,
        TRUE, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        __atomic_sub_fetch(&(Stack->size), 1, __ATOMIC_SEQ_CST);
        return oldTop;
    }
    else
        return NULL;
}

int ULFS_pop() {
    stackNode *rn;
    int retVal;
    
    while (TRUE) {
        rn = TryPop();
        if(rn == EMPTY_STACK)
            return -1;
        if(rn != NULL) {
            retVal = rn->productID;
            // !!! CAUTION !!!
                free(rn);
            // ~~~~~~~~~~~~~~~
            return retVal;
        }
        else Backoff();
    }
}

int ULFS_getSize() {
    int cnt=0;
    stackNode *tmp = Stack->Top;

    while(tmp) {
        ++cnt;
        tmp = tmp->next;
    }
    
    return cnt;
}

int ULFS_getSize2() {
    // return Stack_ptr->size;
    return __atomic_load_n(&Stack->size, __ATOMIC_SEQ_CST);
}

void ULFS_printStack() {
    stackNode *tmp = Stack->Top;
    int i=0;
    
    printf("\nTop to Bottom:\n");
    
    while (tmp->next) {
        
        printf("Node[%d] = %d\n", i++, tmp->productID);
        tmp = tmp->next;
    }
    if(!i)
        printf("Stack is empty..\n"); 
    
    printf("~~~~~~~~~~~~~~\n\n");
}

// ~~~~ ABA Free Implementation ~~~~
// Incomplete... & wrong(in terms of lang syntax)
int TryPush2(stackNode *newNode){
    ULFStack next, oldStack; 
    
    // Assign Stack_
    __atomic_load(Stack, &oldStack, __ATOMIC_SEQ_CST);
    
    newNode->next = oldStack.Top;
    next.Top = newNode;
    next.tag = oldStack.tag+1;

    if(__atomic_compare_exchange(Stack, &oldStack, &next,
        TRUE, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
        return TRUE;
    else return FALSE;
}

int TryPop2() {
    return FALSE;
}

