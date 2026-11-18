/*************************************\
 *  /\_/\    Clara GORBEAU     /\_/\  *
 * ( o.o )      @}}>---       ( o.o ) *
 *  > ^ <    (c) 2025-2026     > ^ <  *
\*************************************/

/**
 * Various functions
 */

#ifndef LIB_UTILS_H
#define LIB_UTILS_H
#include <stddef.h>
#include <stdbool.h>
#include "types.h"

#define SWAP(x, y) do { typeof(x) SWAP = x; x = y; y = SWAP; } while (0)


/**
 * Count the number of in lines in the given file
 * @param const char *filename - file path
 *
 * @return number of lines in file, -1 if I/O error produced
 */
int countlines(const char *filename);



/**
 * Generate random permutation
 * @param unsigned output array
 * @param n array size
 */
void randpermutation(permutation_t *output, int n);


/**
 * randomize a permutation
 * @param unsigned output array
 * @param n array size
 *
 */
void randomizepermutation(permutation_t *output, int n);

/**
 * Test if given array is permutation
 * @param array - array for test
 * @param size array size;
 * @return True if array is permutation
 */
bool permutationtest(permutation_t *array, int size);

/**
 * Generate a random value weighted within the normal distribution
 * with the Box–Muller method. To convert with value for value distributed
 * with mean and variance use expression : mean + variance * gaussian_random()
 *
 * @return Normal distributed random value with mean 0
 */
double gaussian_random();

#endif
