#include "HTable.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

// #define TABLE_SIZE N // Where N is prime
#define TRUE 1
#define FALSE 0


int TABLE_SIZE;
int TOTAL_TABLES;

typedef struct HTNode {
	int key;        // productID
	pthread_mutex_t lock;
	int deleted; 	// init as FALSE
} HTNode;

typedef struct HTStat {
	int no_elems;        // productID
    int key_sum;
} HTStat;

HTNode **HTArray=NULL;
HTStat *HTStatArray=NULL;


// `````````````````````````````````````````````````````````
// As seen in geeksforgeeks.org (Modified to fit my needs though)
// url: https://www.geeksforgeeks.org/program-to-find-the-next-prime-number/

// Function that returns true if n
// is prime else returns false
int isPrime(int n)
{
    // Corner cases
    if (n <= 1)  return FALSE;
    if (n <= 3)  return TRUE;

    // This is checked so that we can skip
    // middle five numbers in below loop
    if (n%2 == 0 || n%3 == 0) return FALSE;

    for (int i=5; i*i<=n; i=i+6)
        if (n%i == 0 || n%(i+2) == 0)
    return FALSE;

    return TRUE;
}

// Function to return the smallest
// prime number greater than N
int nextPrime(int N)
{
    // Knowing that N = even

    int prime = N+1; 
    int found = FALSE;

    // Making sure prime var is odd
    assert(prime%2 != 0);
    // Loop continuously until isPrime returns
    // true for a number greater than n
    do {
        if (isPrime(prime))
            found = TRUE;
        else
            prime = prime+2;

    } while (!found);
    

    return prime;
}
// `````````````````````````````````````````````````````````

// function H1(key) // our Primary hash function
// function H2(key) // our Secondary hash function
// Square & Digit Folding Method Combination
int H1(int key) {
    int hash=0;
    int x;

    x = key*key;
    while (x>0) {
        hash += (x%10); 
        x = x/10;
    }
    
    return hash;
}

// Multiplication Method
int H2(int key) {
    int hash;
    const double A = 0.357859; // is prime

    hash = (int) floor(TABLE_SIZE*(
        fmod(((double)key) * A, 1.00)));
    
    return hash;
}

int Delta(int key) {
    int delta;

    delta = H2(key) % TABLE_SIZE;

    if(delta==0)
        delta = 1;

    return delta;
}

int P(int key, int x) {
    return Delta(key) * x;
    //return key+x; // linear probing for debug..
}

void swap(int *a, int *b) {
    int *tmp;
    tmp = a;
    a = b;
    b = tmp;
    return;
}

void HTInit(int N) {
    int i,j;

    TABLE_SIZE = nextPrime(4*N);
    TOTAL_TABLES = N/3;

    HTArray = malloc(sizeof(HTNode*)*TOTAL_TABLES);
    assert(HTArray);

    HTStatArray = malloc(sizeof(HTStat)*TOTAL_TABLES);
    assert(HTStatArray);
    for (i=0; i<TOTAL_TABLES; i++) {
        HTStatArray[i].key_sum = 0;
        HTStatArray[i].no_elems = 0;
    }

    for (i=0; i<TOTAL_TABLES; i++) {
        HTArray[i] = calloc(TABLE_SIZE,sizeof(HTNode));
        assert(HTArray[i]);
        for(j=0; j<TABLE_SIZE;j++) {
            HTArray[i][j].key = -1;
        }
    }

}

void HTDestroy() {
    assert(HTArray);
    int i;

    for (i=0; i<TOTAL_TABLES; i++) {

        free(HTArray[i]);

    }
    free(HTArray);
}

int HTInsert(int key, int tbl_id) {

    HTNode *hashTable = HTArray[tbl_id];
    int index, keyHash, x=1;
    int o_key = key, tmp_key; // original key
    assert(hashTable);
    assert(HTGetTableStats(NO_ELEMS, tbl_id) < TABLE_SIZE);

	keyHash = H1(key) % TABLE_SIZE;
    index = keyHash;

	// try until node locks
	pthread_mutex_lock(&hashTable[index].lock);

	// go through hashTable by Prob step 
	while (hashTable[index].key != -1) {

		if(key < hashTable[index].key) { // ordered hashing

            if(hashTable[index].deleted == TRUE) {
                hashTable[index].key = key;

                // Update table stats
                // HTStatArray[tbl_id].key_sum += o_key;
                // ++HTStatArray[tbl_id].no_elems;
                __atomic_add_fetch(&HTStatArray[tbl_id].key_sum, 
                o_key, __ATOMIC_SEQ_CST);
                __atomic_add_fetch(&HTStatArray[tbl_id].no_elems, 
                1, __ATOMIC_SEQ_CST);

                pthread_mutex_unlock(&hashTable[index].lock);
                return TRUE;
            }

			// swap(&(hashTable[index].key), &key);
            tmp_key = hashTable[index].key;
            hashTable[index].key = key;
            key = tmp_key;

            pthread_mutex_unlock(&hashTable[index].lock);

		}
		else if(key == hashTable[index].key) {
			
            if(hashTable[index].deleted == TRUE) {

                hashTable[index].deleted = FALSE;
                
                // Update table stats
                // HTStatArray[tbl_id].key_sum += o_key;
                // ++HTStatArray[tbl_id].no_elems;
                __atomic_add_fetch(&HTStatArray[tbl_id].key_sum, 
                o_key, __ATOMIC_SEQ_CST);
                __atomic_add_fetch(&HTStatArray[tbl_id].no_elems, 
                1, __ATOMIC_SEQ_CST);
                
                pthread_mutex_unlock(&hashTable[index].lock);
			    return TRUE;
            }
                
			pthread_mutex_unlock(&hashTable[index].lock);
			return FALSE;

		}
        // Case where key > ht[index].key
        pthread_mutex_unlock(&hashTable[index].lock);
        
		// next position in probe sequence
		index = (keyHash + P(key,x)) % TABLE_SIZE;
		x += 1;
        // lock next
        pthread_mutex_lock(&hashTable[index].lock);
	}

	// insertion
	hashTable[index].key = key;
	hashTable[index].deleted = FALSE;

    // Update table stats
    // HTStatArray[tbl_id].key_sum += o_key;
    // ++HTStatArray[tbl_id].no_elems;
    __atomic_add_fetch(&HTStatArray[tbl_id].key_sum, 
    o_key, __ATOMIC_SEQ_CST);
    __atomic_add_fetch(&HTStatArray[tbl_id].no_elems, 
    1, __ATOMIC_SEQ_CST);

	pthread_mutex_unlock(&hashTable[index].lock);

	return TRUE;
}

int HTSearch(int key, int tbl_id) {
	
    HTNode *hashTable = HTArray[tbl_id];
    int index, keyHash, x=1;
    assert(hashTable);

	keyHash = H1(key) % TABLE_SIZE;
    index = keyHash;

	// try until node locks
	pthread_mutex_lock(&hashTable[index].lock);

	while((hashTable[index].key != -1) && 
		(hashTable[index].key < key)) {
        
        // not node in question, unlock node
        pthread_mutex_unlock(&hashTable[index].lock);
		
        // next position in probe sequence
		index = (keyHash + P(key,x)) % TABLE_SIZE;
		x += 1;
        // lock next
        pthread_mutex_lock(&hashTable[index].lock);
	}

	if((hashTable[index].key == key) &&
		(hashTable[index].deleted == FALSE)) {
		
		pthread_mutex_unlock(&hashTable[index].lock);
		return TRUE;
	}
	else {
        pthread_mutex_unlock(&hashTable[index].lock);
		return FALSE;
	}
}

int HTDelete(int key, int tbl_id) {
	
    HTNode *hashTable = HTArray[tbl_id];
    int index, keyHash, x=1;
    int o_key = key; // original key
    assert(hashTable);


	keyHash = H1(key) % TABLE_SIZE;
    index = keyHash;

	// try until node locks
	pthread_mutex_lock(&hashTable[index].lock);

	while((hashTable[index].key != -1) && 
		(hashTable[index].key < key)) {
        
        // not goal node, unlock node
        pthread_mutex_unlock(&hashTable[index].lock);
		
        // next position in probe sequence
		index = (keyHash + P(key,x)) % TABLE_SIZE;
		x += 1;
        // lock next
        pthread_mutex_lock(&hashTable[index].lock);
	}

	if((hashTable[index].key == key) &&
		(hashTable[index].deleted == FALSE)) {
		
        hashTable[index].deleted = TRUE;

        // Update table stats
        // HTStatArray[tbl_id].key_sum -= o_key;
        // --HTStatArray[tbl_id].no_elems;
        __atomic_sub_fetch(&HTStatArray[tbl_id].key_sum, 
        o_key, __ATOMIC_SEQ_CST);
        __atomic_sub_fetch(&HTStatArray[tbl_id].no_elems, 
        1, __ATOMIC_SEQ_CST);

		pthread_mutex_unlock(&hashTable[index].lock);
		return TRUE;
	}
	else {
        pthread_mutex_unlock(&hashTable[index].lock);
		return FALSE;
	}
}

int HTGetNoElems(int tbl_id) {
    int i,cnt=0;
    for (i=0; i<TABLE_SIZE; i++) {
        if((HTArray[tbl_id]->key != -1) &&
            (HTArray[tbl_id]->deleted=FALSE)){
            ++cnt;
        }
    }
    return cnt;
}

int HTGetKeysum(int tbl_id) {
    int i,sum=0;
    for (i=0; i<TABLE_SIZE; i++) {
        if((HTArray[tbl_id]->key != -1) &&
            (HTArray[tbl_id]->deleted=FALSE)){
            sum += HTArray[tbl_id]->key;
        }
    }
    return sum;
}

int HTGetTableStats(stat_mode mode, int tbl_id) {
    
    return (mode) ? HTStatArray[tbl_id].no_elems : HTStatArray[tbl_id].key_sum;
}

void HTPrintAllTAbles() {
    int i,j;
    for (i=0; i<TOTAL_TABLES; i++) {
        fprintf(stdout, "~~~ HTable %d ~~~\n\n", i);
        for (j=0; j<TABLE_SIZE; j++) {
            if(!(HTArray[i][j].deleted))
                fprintf(stdout, "HT[%d].productID = %d\n", i, 
                        HTArray[i][j].key);
            else
                fprintf(stdout, "HT[%d].productID = %d del\n", i, 
                        HTArray[i][j].key);
        }
        fprintf(stdout, "~~~ END of TABLE ~~~\n\n");
    }
}

void HTPrintTable(int tbl_id) {
    int i;
    fprintf(stdout, "~~~ HTable %d ~~~\n\n", tbl_id);
    for (i=0; i<TABLE_SIZE; i++) {
        if(!(HTArray[tbl_id][i].deleted))
            fprintf(stdout, "HT[%d].productID = %d\n", 
                    tbl_id, HTArray[tbl_id][i].key);
        else
            fprintf(stdout, "HT[%d].productID = %d del\n", 
                    tbl_id, HTArray[tbl_id][i].key);
    }
    fprintf(stdout, "~~~ END of TABLE ~~~\n\n");
}