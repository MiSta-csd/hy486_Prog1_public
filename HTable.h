#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <limits.h>

typedef struct HTNode HTNode;

typedef struct HTStat HTStat;

extern HTNode **HTArray;
extern HTStat *HTStatArray;

typedef enum stat_mode {
    KEY_SUM     = 0,
    NO_ELEMS    = 1
} stat_mode;

void HTInit(int N);

int HTSearch(int key, int t_id);

int HTInsert(int key, int t_id);

int HTDelete(int key, int t_id);

void HTDestroy(void);

int HTGetTableStats(stat_mode mode, int t_id);

void HTPrintAllTAbles();

int HTGetNoElems(int tbl_id);

void HTPrintTable(int tbl_id);