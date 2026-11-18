/*************************************\
 *  /\_/\    Clara GORBEAU     /\_/\  *
 * ( o.o )      @}}>---       ( o.o ) *
 *  > ^ <    (c) 2025-2026     > ^ <  *
\*************************************/

#ifndef LIB_CETSP_H
#define LIB_CETSP_H
#include <stddef.h>
#include <stdbool.h>

#include "types.h"

/*
 * CETSP point
 */
typedef struct cetsp_point{
    coord_t x;
    coord_t y;
    coord_t z;
    coord_t ra;
    coord_t rb;
}cetsp_point;


/*
 * CETSP solution set
 */
typedef struct cetsp_solution{
    long        size;
    int        *path; // sequence
    eu_point   *tp;   //way points
}cetsp_solution;

/*
 * CETSP-Set structure
 */
typedef struct cetsp_set{
    long          size;//set size
    long          depotidx;//depart point index TODO TO DELETE
    eu_point      depot;//depart point
    cetsp_point  *point;
}cetsp_set;

/**
 * Initialize CETSP-Set object from file
 * @param const char *filename - file name
 * @return cetsp_set - CETSP structure
 */
cetsp_set *load_cetsp(const char *filename);


/**
 * Free CETSP structure
 * @param cetsp_set CETSP structure
 */
void free_cetsp_set(cetsp_set *);


/**
 * Initialize CETSP solution structure and set all points to 0
 * @param long solution size;
 *
 * @return cetsp_set - CETSP solution structuree
 */
cetsp_solution * init_cetsp_sol(long size);

/**
 * Initialize CETSP solution structure from file
 * @param filename - file name
 *
 * @return cetsp_solution - CETSP solution structure
 */
cetsp_solution * load_cetsp_sol(const char *filename);

/**
 * Load and check sequence of a CETSP solution
 * @param seq - resulted sequence
 * @param set CETSP set
 * @param filename - file name
 * @return true if solution is feasible
 */
bool load_cetsp_sequence(permutation_t *seq, cetsp_set *set, const char *filename, double gap);

/**
 * Free CETSP solution structure
 * @param cetsp_set CETSP solution structure
 */
void free_cetsp_sol(cetsp_solution *);

/**
 * Calculate the solution euclidien distance by turning points
 *
 * @param cetsp_sol* cetsp solution structure
 * @return double solution euclidien distance
 */

distance_t cetsp_sol_eudist(cetsp_solution *);


/**
 * Remove points which have the depot in theirs wake-up neighborhood
 *
 * @param CETSP structure
 * @return new size;
 */
long cetsp_cleanup(cetsp_set * set);

/**
 * Getting the grid of the instance
 *
 * @param CETSP structure
 *
 * @param x1 lower x coordinate
 * @param x2 upper x coordinate
 * @param y1 lower y coordinate
 * @param y2 upper y coordinate
 * @param z1 lower z coordinate
 * @param z2 upper z coordinate
 *
 */
void cetsp_getgrid(cetsp_set *set, double *x, double *y, double *z);


/**
 * Save CETSP instance to file
 * @param outfile - output filehandler
 * @param set  - CETSP instance
 * @return true if file was saved
 */
bool cetsp_save(FILE *outfh, cetsp_set *cset);

#endif
