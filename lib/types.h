/*************************************\
 *  /\_/\    Clara GORBEAU     /\_/\  *
 * ( o.o )      @}}>---       ( o.o ) *
 *  > ^ <    (c) 2025-2026     > ^ <  *
\*************************************/

/*
 * Basic types
 */
#ifndef CL_TYPES_H
#define CL_TYPES_H

typedef double coord_t;
typedef double distance_t;

typedef int permutation_t;

/*
 * Euclidien point
 */
typedef struct eu_point{
    coord_t x;
    coord_t y;
    coord_t z;
}eu_point;


#endif
