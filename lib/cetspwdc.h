/************************************\
*  /\_/\    Clara GORBEAU     /\_/\  *
* ( o.o )      @}}>---       ( o.o ) *
*  > ^ <    (c) 2025-2026     > ^ <  *
\************************************/

/*
 * CETSP-WDC structure and function library
 */
#ifndef LIB_CETSPWDCLIB_H
#define LIB_CETSPWDCLIB_H
#include <stdio.h>
#include <stdbool.h>
#include "cetsp.h"

/*
 * CETSP-WDC data node
 * Version 3.0
 */
typedef struct wdc_node{
    double x; // coordinates
    double y;
    double z;
    double rw;// wake-up distance
    double rc;// communication distance
    double wakeuptime; // wake-up time
    double commtime;//communication time

}wdc_node;

/*
 * CETSP-WDC data set
 */
typedef struct wdc_set{
    long      size;
    long      depotidx;//depot index
    eu_point  depot;//depot point
    wdc_node *node;
}wdc_set;


/*
 * CETSP-WDC solution's slots structure
 * Version 2.0
 */
typedef struct wdc_solution_point{
    //wake-up start point
    double Ax;
    double Ay;
    // coord_t Az;
    //communication start point
    double Bx;
    double By;
    // coord_t Bz;
    //communication finish point
    double Cx;
    double Cy;
    // coord_t Cz;
    //distances
    coord_t AB;
    coord_t BC;
    //times
    double wakeuptime;
    double commtime;
    // double dist;//effective distance
    long nodeindex;//index of node
}wdc_solution_point;

/*
 * CETSP-WDC solution set
 * Version 3.0
 */
typedef struct wdc_solution{
    long size;
    long removed;//number of removed points
    double speed;
    double distance;
    double actiontime; // time with actions
    double totaltime; // time without any action
    double gaintime;//time gained while collect on flight
    wdc_solution_point *wp;
}wdc_solution;


/**
 * Initialisation of an empty CETSP-WDC solution
 * @return empty CETSP-WDC solution structure
 */
wdc_solution *init_wdc_solution(); //version 2.0

/**
 * Free CETSP solution structure
 * @param sol CETSP-WDC solution structure
 */
void free_wdc_solution(wdc_solution *sol);//version 2.0

/**
 * Save CETSP-WDC solution to file
 * @param outfile - output filehandler
 * @param sol - CETSP-WDCsolution instance
 * @return true if file was saved
 */
bool wdc_solution_save(FILE *outfh, wdc_solution *sol);




/**
 * Initialization CETSP-WDC data from cetspr file
 * @param infile - filename
 *
 * @return CETSP-WDC structure
 *
 */
wdc_set *load_cetspwdc(const char *infile);

/**
 * Free CETSP-WDC data structure
 * @param sol CETSPR data structure
 */
void free_wdc_set(wdc_set *wdcset);

/**
 * Save CETSP-WDC instance to file
 * @param outfile - output filehandler
 * @param set  - CETSP-WDC instance
 * @return true if file was saved
 */
bool wdc_save(FILE *outfh, wdc_set *wdcset);


/**
 * Remove points which have the depot in theirs wake-up neighborhood
 *
 * @param CETSP-WDC structure
 * @return new size;
 */
long cetspwdc_cleanup_rw(wdc_set * set);

/**
 * Remove points which have the depot in theirs communication neighborhood
 *
 * @param CETSP-WDC structure
 * @return new size;
 */
long cetspwdc_cleanup_rc(wdc_set * set);



/**
 * Copy CETSP-WDC dataset
 * @param src CETSP-WDC set
 * @return newly initialized copy of CETSP-WDC set
 */
wdc_set *copy_wdc_set(wdc_set *src);


/**
 * Remove wake-up zone and time
 * @param rset CETSP-WDC set to proceed
 */
void wdc_remove_wakeup(wdc_set *rset);


/**
 * Remove wakeup zone and set it as the communication zone
 * @param rset CETSP-WDC set to proceed
 */
void wdc_remove_wakeupzone(wdc_set *rset);

/**
 * Compute solution time
 * @param sol CESPR-WC solution
 * @param speed cruiser speed
 */
double wdc_solution_time(wdc_solution *sol, double speed);

/**
 * Compute solution time by points with given order
 * @param pnt CESPR-WC solution points
 * @param seq sequence
 * @param solution size
 * @param speed cruiser speed
 */
double wdc_solutionpoints_time(wdc_solution_point *pnt, permutation_t *seq, int size, double speed);



/**
 * Homogenious instances generator
 * @param ceset CETSP set
 * @return new WDC set
 */
wdc_set * init_from_cetsp_fix(cetsp_set *ceset, double rwrc);


/**
 * Generate new instance by Behdani procedure
 * @return new WDC set
 */
wdc_set * init_generate_behdani(const long nbnode, int class, double rwrc, double wakeuptime, double commtime);


/**
 * Generate new instance from CETSP instance
 * @return new WDC set
 */
wdc_set * init_generate_cetsp(cetsp_set *ceset, double rcrw, double wakeuptime, double commtime);



/**
 * Heterogenius instances generator
 * @param ceset CETSP set
 * @return new WDC set
 */
wdc_set * init_from_cetsp_rnd(cetsp_set *ceset, bool randomizebase);


#endif
