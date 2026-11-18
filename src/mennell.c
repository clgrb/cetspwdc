/*************************************\
 *  /\_/\    Clara GORBEAU     /\_/\  *
 * ( o.o )      @}}>---       ( o.o ) *
 *  > ^ <    (c) 2025-2026     > ^ <  *
\*************************************/
/**
 * Mennel's instances based generator
 *
 * Please download the base instance from
 *
 * Mennell, W., 2009. Heuristics for rolving three routing problems:
 * Close-Enough Traveling Salesman P*roblem, Close-Enough Vehicle Routing
 * Problem, Sequence-Dependent Team Orienteering Problem. Ph.D. thesis.
 * University of Maryland, College Park, MD, USA.
 * URL: https://hdl.handle.net/1903/9822.
 */
#define _GNU_SOURCE

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <argp.h>
#include <err.h>
#include <time.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"
#include "cetsp.h"
#include "cetspwdc.h"

static struct stat st = {0};

static int generateinstance(const char *cetspfilename){
    char fullname[FILENAME_MAX];
    sprintf(fullname, "%s/%s", MENNELL_CETSP_DIR, cetspfilename);
    if (access(fullname, R_OK) != 0) {
        warn("Failed to open file %s", fullname);
        return 0;
    }

    cetsp_set *set =  load_cetsp(fullname);
    if (set == NULL || set->size < 1){
        warn("Failed to open CESTP file %s", fullname);
        return 0;
    }
    double x, y, z; //grid size
    cetsp_getgrid(set, &x, &y, &z);

    printf( "Found cetsp file: %s, %ld nodes, grid:(%.2fx%.2f)\n", cetspfilename, set->size, x, y);
    double maxdimension = (y > x) ? y : x;

    wdc_set *wdcset = init_generate_cetsp(set, 0.5, 1,  maxdimension/16);

    sprintf(fullname, "%s/%swdc", MENNELL_WDC_DIR, cetspfilename);
    FILE *out = fopen(fullname, "w");
    wdc_save(out, wdcset);
    fclose(out);

    free_wdc_set(wdcset);
    free_cetsp_set(set);
    return 1;
}

int main(){
    int count = 0;
    struct dirent* hFile;

    srand48(GENERATOR_SEED);


    //create directory if not exists
    if (stat(MENNELL_WDC_DIR, &st) == -1)
        mkdir(MENNELL_WDC_DIR, 0755);

    DIR* dirFile = opendir(MENNELL_CETSP_DIR);
    if (dirFile == NULL){
        warn("Could not open CESP instances directory %s", MENNELL_CETSP_DIR);
        exit(EXIT_FAILURE);
    }

    printf("Processing *.cetsp files in directory %s\n", MENNELL_CETSP_DIR);

    while (( hFile = readdir( dirFile )) != NULL ) {
       if ( strstr( hFile->d_name, ".cetsp" ))
           count += generateinstance(hFile->d_name);

    }
    printf("Processed %d files\n", count);

    closedir(dirFile);

    puts("***Finished***");
    return EXIT_SUCCESS;
}
