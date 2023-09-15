#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <limits.h>


int ULFS_pop();

void ULFS_push(int key);

int ULFS_getSize();

void ULFS_init();

void ULFS_destroy();

int ULFS_getSize2();

void ULFS_printStack();