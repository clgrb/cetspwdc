/*************************************\
 *  /\_/\    Clara GORBEAU     /\_/\  *
 * ( o.o )      @}}>---       ( o.o ) *
 *  > ^ <    (c) 2025-2026     > ^ <  *
\*************************************/


#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include "utils.h"

/**
 * Count the number of in lines in the given file
 * @param const char *filename - file path
 *
 * @return number of lines in file
 */
int countlines(const char *filename){
    int chr;
    int lines = 0;
    FILE *stream = fopen(filename, "r");
    if (stream == NULL){
        warn("Error reading %s", filename);
        return -1;
    }

    while ((chr = fgetc(stream)) != EOF){
        if (chr == '\n'){
            lines++;
        }
    }

    fclose(stream);
    return lines;
}


/**
 * Generate random permutation
 * @param unsigned output array
 * @param n array size
 */
void randpermutation(permutation_t *a, int n){
    permutation_t i, j, t;
    for(i=0; i < n; i++){
        a[i] = i;
    }
    i = n;
    while (i>1){
        i--;
        j = lrand48() % i;
        //swap
        t = a[i];
        a[i] = a[j];
        a[j] = t;
    }
}

/**
 * randomize a permutation
 * @param unsigned output array
 * @param n array size
 *
 */
void randomizepermutation(permutation_t *a, int n){
    permutation_t i, j, t;
    i = n;
    while (i>1){
        i--;
        j = lrand48() % i;
        //swap
        t = a[i];
        a[i] = a[j];
        a[j] = t;
    }
}


/**
 * Permutation test comparator
 */
static int perm_comp(const void *a, const void *b) {
    return (*(permutation_t *)a - *(permutation_t *)b);
}
/**
 * Test if given array is permutation
 * @param p - unsigned array for test
 * @param size array size;
 * @return True if array is a permutation
 */
bool permutationtest(permutation_t *p, int size){
    if (size < 1)
        return false;
    bool result = true;
    permutation_t *temp = malloc(sizeof(permutation_t) * size);
    memcpy(temp, p, sizeof(permutation_t) * size);

    qsort(temp, size, sizeof(permutation_t), perm_comp);
    permutation_t d = temp[0];
    for (int i=1; i < size; i++){
        if (temp[i] <= d){
            result = false;
            break;
        }
        d = temp[i];
    }
    free(temp);
    return result;
}




// generate a random value weighted within the normal distribution using Marsaglia polar method
double gaussian_random() {
    static bool issaved = false;
    static double savedval;

    if(issaved){
        issaved = false;
        return savedval;
    }

    double v, u, s;

    do {
        u = 2 * drand48() - 1; // random in interval[-1; 1]
        v = 2 * drand48() - 1; // random in interval[-1; 1]
        s = u *u + v * v;
    }while(s >= 1.0 || s == 0.0);
    s = sqrt(-2.0 * log(s) / s);

    savedval = v * s;
    issaved = true;
    return u * s;
}
