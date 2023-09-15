#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>


void DLLInit(void);

int DLLSearch(int key);

int DLLInsert(int key);

int DLLDelete(int key);

void DLLDestroy(void);

// returns list size
int DLLSize(void);
// return KeySum
int DLLKeySum(void);