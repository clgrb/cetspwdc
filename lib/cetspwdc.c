/*************************************\
 *  /\_/\    Clara GORBEAU     /\_/\  *
 * ( o.o )      @}}>---       ( o.o ) *
 *  > ^ <    (c) 2025-2026     > ^ <  *
\*************************************/

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include <err.h>

#include "cetspwdc.h"
#include "utils.h"

#ifdef WIN32
    #include <io.h>
    #define F_OK 0
    #define access _access
#else
    #include <unistd.h>
#endif

#define LINE_SIZE 256
#define HOVER_DISTANCE 0.01

/**
 * Load CETSP-WDC data line
 * @param wdc_node *item pointer to store value
 *
 * @return true if line is valid
 *
 */
inline static bool loadCETSPWDCLine(wdc_node *item, char *line){
    char *el;
    const char *delim = " ";
    char *result = NULL;

    double volatile *values[] = {
        &item->x, &item->y, &item->z,
        &item->rw, &item->wakeuptime,
        &item->rc, &item->commtime
    };

    long i;
    const long limit = sizeof(values) / sizeof(double*);

    el = strtok(line, delim);
    for(i=0; i < limit && el !=NULL; i++){
        *values[i] = strtod(el, &result);
        if (result == el)
            break;
        el = strtok(NULL, delim);
    }
    //something wrong
    if (i < limit)
        return false;

    return true;
}

static double randinterval(double a, double b){
    return round(100 *(a + (b - a) * drand48())) / 100;
}

/**
 * Initialisation of an empty CETSP-WDC solution
 * @return empty CETSP-WDC solution structure
 */
wdc_solution *init_wdc_solution(){
    wdc_solution *rs = malloc(sizeof(wdc_solution));
    rs->size = 0;
    rs->speed = 0.0;
    rs->wp = NULL;
    return rs;
}


/**
 * Free CETSP-WDC solution structure
 * @param sol CETSP-WDC solution structure
 */
void free_wdc_solution(wdc_solution *sol){
    if (sol == NULL)
        return;

    if (sol->wp != NULL)
        free(sol->wp);

    free(sol);
}



wdc_set *load_cetspwdc(const char *infile){
    char buf[LINE_SIZE];

    if (access(infile, F_OK) != 0) {
        warn("Couldnot open file %s", infile);
        return NULL;
    }

    long lines = countlines(infile);

    if (lines <= 0){
        fprintf(stderr, "File %s is empty", infile);
        return NULL;
    }

    wdc_set *rs = malloc(sizeof(wdc_set));
    if (rs == NULL)
        err(EXIT_FAILURE, "Out of memory");

    rs->size = 0;
    rs->depotidx = 0;

    rs->node = malloc(sizeof(wdc_node) * (lines + 1));
    if (rs->node == NULL)
        err(EXIT_FAILURE, "Out of memory INIT_POINT");

    FILE *stream = fopen(infile, "r");

    /*
     * Firstly, we need to check if the depot point is in file. Since, it can
     * be located at the end of the file and must be placed at the top, we must
     * process that point before handling the others.
     */
    char *line, *el;
    const char *depot = "//Depot: ";

    while (fgets(buf, LINE_SIZE, stream)){
        if (strncasecmp(buf, depot, strlen(depot)) != 0){
            continue;
        }
        line = &buf[strlen(depot)]; // just change pointer
        //first value X
        if ((el = strtok(line, ", ")) == NULL)
            continue;

        rs->depot.x = strtod(el, NULL);

        //second value Y
        if ((el = strtok(NULL, ", ")) == NULL)
            continue;

        rs->depot.y = strtod(el, NULL);

        rs->depot.z = 0;
        rs->node[rs->depotidx].x = rs->depot.x;
        rs->node[rs->depotidx].y = rs->depot.y;
        rs->node[rs->depotidx].z = rs->depot.z;
        rs->size++;
        break; //we take only one point
    }

    //Handling other points
    fseek(stream, 0, SEEK_SET);

    while (fgets(buf, LINE_SIZE, stream)){
        size_t len = strlen(buf);

        //skip the comments and empty lines
        if (len < 2 || strncmp(buf, "//", 2) == 0){
            continue;
        }

        //TODO if line is too long
        if (len && buf[len-1] != '\n') {
            // TODO
        }

        if (loadCETSPWDCLine(&rs->node[rs->size], buf)){
            rs->size++;
        }
    }
    fclose(stream);

    //force depot's radii to be 0 TODO remove
    rs->node[rs->depotidx].rw = 0;
    rs->node[rs->depotidx].rc = 0;
    rs->node[rs->depotidx].commtime = 0;
    rs->node[rs->depotidx].wakeuptime = 0;

    rs->node = realloc(rs->node, sizeof(wdc_node) * rs->size);

    return rs;
}


void free_wdc_set(wdc_set *wdcset){
    if (wdcset == NULL)
        return;
    free(wdcset->node);
    wdcset->node = NULL;
    free(wdcset);
    wdcset = NULL;
}


bool wdc_save(FILE *outfh, wdc_set *wdcset){
    if( outfh == NULL){
        return false;
    }

    fprintf(outfh, "//CETSP-WDC data set\n");
    fprintf(outfh, "//x y z wakeupdist wakeuptime commdist commtime\n");

    for(int i=0; i < wdcset->size; i++ ){
        if (i == wdcset->depotidx)
            continue;

        fprintf(outfh, "%.4f %.4f %.4f %.4f %.4f %.4f %.4f\n",
                wdcset->node[i].x,
                wdcset->node[i].y,
                wdcset->node[i].z,
                wdcset->node[i].rw,
                wdcset->node[i].wakeuptime,
                wdcset->node[i].rc,
                wdcset->node[i].commtime
        );
    }

    //DEPOT point output
    if (wdcset->depotidx >= 0){
        fputs("\n", outfh);
        fprintf(outfh, "//Depot: %.4f, %.4f, %.4f\n",
            // wdcset->node[wdcset->depotidx].x,
            // wdcset->node[wdcset->depotidx].y,
            // wdcset->node[wdcset->depotidx].z
            wdcset->depot.x,
            wdcset->depot.y,
            wdcset->depot.z
        );
    }
    return true;
}

/**
 * Save CETSP WDC solution to file
 * @param outfile - output filehandler
 * @param sol - CETSP-R solution instance
 * @return true if file was saved
 */
bool wdc_solution_save(FILE *outfh, wdc_solution *sol){
    if( outfh == NULL){
        return false;
    }
    fprintf(outfh, "node;x;y;speed;actions\n");
    fprintf(outfh, "%d;%.3f;%.3f;%.2f;TAKE-OFF\n", 0, sol->wp[0].Cx, sol->wp[0].Cy, sol->speed);
    for (long i=1; i < sol->size; i++){

        if (sol->wp[i].wakeuptime > 0){
            double AB = hypot(sol->wp[i].Ax - sol->wp[i].Bx, sol->wp[i].Ay - sol->wp[i].By);
            if (AB < HOVER_DISTANCE){
                fprintf(outfh, "%ld;%0.3f;%0.3f;%0.2f;WAKEUP %ld, HOVER %.2f\n",
                    sol->wp[i].nodeindex, sol->wp[i].Ax, sol->wp[i].Ay, 0.0,
                    sol->wp[i].nodeindex, sol->wp[i].wakeuptime );
            }
            if (AB > sol->speed * sol->wp[i].wakeuptime){
                fprintf(outfh, "%ld;%0.3f;%0.3f;%0.2f;WAKEUP %ld\n",
                    sol->wp[i].nodeindex, sol->wp[i].Ax, sol->wp[i].Ay, sol->speed,
                    sol->wp[i].nodeindex );
            }
            else{
                fprintf(outfh, "%ld;%0.3f;%0.3f;%0.2f;WAKEUP %ld\n",
                    sol->wp[i].nodeindex, sol->wp[i].Ax, sol->wp[i].Ay,
                    AB / sol->wp[i].wakeuptime, sol->wp[i].nodeindex);
            }

        }
        if (sol->wp[i].commtime > 0){
            double BC = hypot(sol->wp[i].Bx - sol->wp[i].Cx, sol->wp[i].By - sol->wp[i].Cy);
            if (BC < HOVER_DISTANCE){
                fprintf(outfh, "%ld;%0.3f;%0.3f;%0.2f;COMMUNICATION %ld, HOVER %.2f\n",
                    sol->wp[i].nodeindex, sol->wp[i].Bx, sol->wp[i].By, 0.0,
                    sol->wp[i].nodeindex, sol->wp[i].commtime);
            }
            if (BC > sol->speed * sol->wp[i].commtime){
                fprintf(outfh, "%ld;%0.3f;%0.3f;%0.2f;COMMUNICATION %ld\n",
                    sol->wp[i].nodeindex, sol->wp[i].Bx, sol->wp[i].By, sol->speed,
                    sol->wp[i].nodeindex);
            }
            else{
                fprintf(outfh, "%ld;%0.3f;%0.3f;%0.2f;COMMUNICATION %ld\n",
                    sol->wp[i].nodeindex, sol->wp[i].Bx, sol->wp[i].By,
                    BC / sol->wp[i].commtime, sol->wp[i].nodeindex);

            }
        }
        fprintf(outfh, "%ld;%.3f;%.3f;%0.2f;\n", sol->wp[i].nodeindex, sol->wp[i].Cx, sol->wp[i].Cy, sol->speed);
    }
    fprintf(outfh, "%.3f;%.3f;0;LANDING\n", sol->wp[0].Ax, sol->wp[0].Ay);
    return true;
}


/**
 * Remove wake-up zone and time
 * @param rset CETSP-WDC set to proceed
 */
void wdc_remove_wakeup(wdc_set *rset){
    for(long i=0; i < rset->size; i++){
        rset->node[i].wakeuptime = 0.0;
        rset->node[i].rw = rset->node[i].rc;
    }
}

/**
 * Remove wakeup zone and set it as the communication zone
 * @param rset CETSP-WDC set to proceed
 */
void wdc_remove_wakeupzone(wdc_set *rset){
    for(long i=0; i < rset->size; i++){
        rset->node[i].rw = rset->node[i].rc;
    }
}


/**
 * Remove points which have the depot in theirs wake-up neighborhood
 *
 * @param CETSP-WDC structure
 * @return new size;
 */
long cetspwdc_cleanup_rw(wdc_set* set){
    long writeptr = 0;
    for(long i=0; i < set->size; i++){
        if (i == set->depotidx){
            set->depotidx = writeptr;
            memcpy(&set->node[writeptr++], &set->node[i], sizeof(wdc_node));
            continue;
        }
        // double dist = hypot(set->node[set->depotidx].x - set->node[i].x, set->node[set->depotidx].y - set->node[i].y);
        double dist = hypot(set->depot.x - set->node[i].x, set->depot.y - set->node[i].y);
        if (dist > set->node[i].rw){
            memcpy(&set->node[writeptr++], &set->node[i], sizeof(wdc_node));
        }
    }
    set->size = writeptr;

    return set->size;
}

/**
 * Remove points which have the depot in theirs communication neighborhood
 *
 * @param CETSP WDC structure
 * @return new size;
 */
long cetspwdc_cleanup_rc(wdc_set *set){
    long writeptr = 0;
    for(long i=0; i < set->size; i++){
        if (i == set->depotidx){
            set->depotidx = writeptr;
            memcpy(&set->node[writeptr++], &set->node[i], sizeof(wdc_node));
            continue;
        }
        // double dist = hypot(set->node[set->depotidx].x - set->node[i].x, set->node[set->depotidx].y - set->node[i].y);
        double dist = hypot(set->depot.x - set->node[i].x, set->depot.y - set->node[i].y);

        if (dist > set->node[i].rc){
            memcpy(&set->node[writeptr++], &set->node[i], sizeof(wdc_node));
        }
    }
    set->size = writeptr;

    return set->size;
}


wdc_set *copy_wdc_set(wdc_set *src){
    wdc_set *rs = malloc(sizeof(wdc_set));
    rs->size = src->size;
    rs->depotidx = src->depotidx;
    memcpy(&rs->depot, &src->depot, sizeof(eu_point));

    size_t nodesize = sizeof(wdc_node) * rs->size;
    rs->node = malloc(nodesize);
    memcpy(rs->node, src->node, nodesize);
    return rs;
}


/**
 * Compute solution time
 * @param sol CESPR-WC solution
 * @param speed cruiser speed
 */
double wdc_solution_time(wdc_solution *sol, double speed){
    double time = 0;
    int pi = sol->size - 1;
    wdc_solution_point *pnt = sol->wp;
    for (int ci = 0; ci < sol->size; ci++){
        //deplace
        time += hypot(pnt[pi].Cx - pnt[ci].Ax, pnt[pi].Cy - pnt[ci].Ay) / speed;
        //wakeup
        // double ABtime = pnt[ci].AB / speed;
        double ABtime = hypot(pnt[ci].Ax - pnt[ci].Bx, pnt[ci].Ay - pnt[ci].Ay) /speed;
        time += (ABtime <= pnt[ci].wakeuptime) ? pnt[ci].wakeuptime : ABtime;
        //communication
        // double BCtime = pnt[ci].BC / speed;
        double BCtime = hypot(pnt[ci].Bx - pnt[ci].Cx, pnt[ci].By - pnt[ci].Cy) /speed;
        time += (BCtime <= pnt[ci].commtime) ? pnt[ci].commtime : BCtime;
        pi = ci;
    }
    return time;
}



/**
 * Compute solution time by points with given order
 * @param pnt CESPR-WC solution points
 * @param seq sequence
 * @param solution size
 * @param speed cruiser speed
 */
double wdc_solutionpoints_time(wdc_solution_point *pnt, permutation_t *seq, int size, double speed){
    double time = 0;
    int pi = seq[size -1];
    for (int i = 0; i < size; i++){
        int ci = seq[i];
        ///moving
        time += hypot(pnt[pi].Cx - pnt[ci].Ax, pnt[pi].Cy - pnt[ci].Ay) / speed;
        //wakeup
        // double ABtime = pnt[ci].AB / speed;
        double ABtime = hypot(pnt[ci].Ax - pnt[ci].Bx, pnt[ci].Ay - pnt[ci].Ay) /speed;
        time += (ABtime <= pnt[ci].wakeuptime) ? pnt[ci].wakeuptime : ABtime;
        //communication
        // double BCtime = pnt[ci].BC / speed;
        double BCtime = hypot(pnt[ci].Bx - pnt[ci].Cx, pnt[ci].By - pnt[ci].Cy) /speed;
        time += (BCtime <= pnt[ci].commtime) ? pnt[ci].commtime : BCtime;
        pi = ci;
    }
    return time;
}


/**
 * Generates a new instance using Behdani procedure
 *
 * @param rwrc Ratio of wake-up radius to communication radius.
 * @param wakeupTime Wake-up time.
 * @param commTime Communication time.
 * @return A new WDC set.
 */
wdc_set * init_generate_behdani(const long nbnode, int class, double rwrc, double wakeuptime, double commtime){
    const double grid = 16.0;
    const double baseradius = 1.0;
    const double class_low[] = {
        0.1, 0.4, 0.7
    };
    const double class_hight[] = {
        0.4, 0.7, 1.0
    };

    wdc_set *wdcset = malloc(sizeof(wdc_set));
    if (wdcset == NULL){
        err(EXIT_FAILURE, "CETSPR: Out of memory");
    }

    wdcset->size = nbnode;
    wdcset->node = malloc(sizeof(wdc_node) * wdcset->size);

    for (long i=0; i < nbnode; i++){
        wdcset->node[i].x = grid * drand48();
        wdcset->node[i].y = grid * drand48();
        wdcset->node[i].z = 0;
        if (i==0){
            wdcset->node[i].rc = 0;
            wdcset->node[i].rw = 0;
            wdcset->node[i].wakeuptime = 0;
            wdcset->node[i].commtime = 0;
        }
        else{
            wdcset->node[i].rc = baseradius * ((class_hight[class] - class_low[class]) * drand48() + class_low[class]);
            wdcset->node[i].rw = rwrc * wdcset->node[i].rc;
            wdcset->node[i].wakeuptime = wakeuptime;
            wdcset->node[i].commtime = commtime;
        }
    }

    wdcset->depotidx = 0;
    wdcset->depot.x = wdcset->node[0].x;
    wdcset->depot.y = wdcset->node[0].y;
    wdcset->depot.z = wdcset->node[0].z;
    return wdcset;
}


/**
 * Generates a new instance from a CETSP instance.
 *
 * @param rwrc Ratio of wake-up radius to communication radius.
 * @param wakeupTime Wake-up time.
 * @param commTime Communication time.
 * @return A new WDC set.
 */
wdc_set * init_generate_cetsp(cetsp_set *ceset, double rwrc, double wakeuptime, double commtime){
    if (ceset == NULL)
        return NULL;

    wdc_set *wdcset = malloc(sizeof(wdc_set));
    if (wdcset == NULL){
        err(EXIT_FAILURE, "CETSP-WDC: Out of memory");
    }

    wdcset->size = ceset->size;
    wdcset->depotidx = ceset->depotidx;
    wdcset->depot.x = ceset->depot.x;
    wdcset->depot.y = ceset->depot.y;
    wdcset->depot.z = ceset->depot.z;

    wdcset->node = malloc(sizeof(wdc_node) * ceset->size);
    for (long i=0; i < ceset->size; i++){
        wdcset->node[i].x = ceset->point[i].x;
        wdcset->node[i].y = ceset->point[i].y;
        wdcset->node[i].z = ceset->point[i].z;

        if (i==ceset->depotidx){
            wdcset->node[i].rc = 0;
            wdcset->node[i].rw = 0;
            wdcset->node[i].wakeuptime = 0;
            wdcset->node[i].commtime = 0;
        }
        else{
            wdcset->node[i].rc = ceset->point[i].ra / rwrc;
            wdcset->node[i].rw = ceset->point[i].ra;
            wdcset->node[i].wakeuptime = wakeuptime;
            wdcset->node[i].commtime = commtime;
        }
    }
    return wdcset;
}



