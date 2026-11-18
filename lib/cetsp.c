/*************************************\
 *  /\_/\    Clara GORBEAU     /\_/\  *
 * ( o.o )      @}}>---       ( o.o ) *
 *  > ^ <    (c) 2025-2026     > ^ <  *
\*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include <err.h>

#include <math.h>

#include "cetsp.h"
#include "utils.h"

#ifdef WIN32
    #include <io.h>
    #define F_OK 0
    #define access _access
#else
    #include <unistd.h>
#endif

#define LINE_SIZE 256
#define PATH_SIZE 512

/**
 * Check for circle and line intersection
 * @param c cirlce centre coordinate
 * @param r circle radius
 * @param a first line point
 * @param b second line point
 * @param &t1 pointer to write the first point
 * @param &t2 pointer to write the second point
 * @return True if circle and line is intersect false otherwise
 */
static bool linecircle_intersection(eu_point *c, double r, eu_point *a, eu_point *b, double *t1, double *t2){
    const double cax = c->x - a->x;
    const double cay = c->y - a->y;
    const double abx = a->x - b->x;
    const double aby = a->y - b->y;

    *t1 = 0;
    *t2 = 0;

    //if line is one point
    if (abx == 0.0 && aby == 0.0){
        return hypot(cax, cay) <= r;
    }

    double A = abx * abx + aby * aby;
    double B = abx * cax + aby * cay;
    double C = cax * cax + cay * cay - r * r;

    if (B * B - A * C >= 0){
        *t1 = (-B + sqrt(B*B - A*C)) / A;
        *t2 = (-B - sqrt(B*B - A*C)) / A;
        return true;
    }
    return false;
}


/**
 * Load data line
 * @param cetsp_point *item pointer to store value
 *
 * @return true if line is valid
 *
 */
inline static bool loadCETSPLine(cetsp_point *item, char *line){
    char *el;
    const char *delim = " ";

    //first value X
    if ((el = strtok(line, delim)) == NULL)
        return false;
    item->x = strtod(el, NULL);

    //second value Y
    if ((el = strtok(NULL, delim)) == NULL)
        return false;
    item->y = strtod(el, NULL);

    //Z
    if ((el = strtok(NULL, delim)) == NULL)
        return false;
    item->z = strtod(el, NULL);

    //first radius is fourth value
    if ((el = strtok(NULL, delim)) == NULL)
        return false;
    item->ra = strtod(el, NULL);

    //and the fifth value is the second radius
    if ((el = strtok(NULL, ", ")) == NULL)
        return false;
    item->rb = strtod(el, NULL);

    return true;
}


/**
 * Initialize CETSP-Set object from file
 * @param const char *filename - file name
 * @return cetsp_set - CETSP structure
 */
cetsp_set *load_cetsp(const char *filename){
    char buf[LINE_SIZE];
    cetsp_set * set;

    if (access(filename, R_OK) != 0) {
        warn("CETSP file %s", filename);
        return NULL;
    }

    long lines = countlines(filename);

    if (lines <= 0){
        warn("Empty file %s", filename);
        return NULL;
    }

    set = malloc(sizeof(cetsp_set));
    if (set == NULL){
        err(EXIT_FAILURE, "Out of memory INIT_CETSP");
    }

    set->point = malloc(sizeof(cetsp_point) * lines);
    if (set->point == NULL){
        err(EXIT_FAILURE, "Out of memory INIT_POINT");
    }

    set->size = 0;
    set->depotidx = -1;
    FILE *stream = fopen(filename, "r");

    /*
    * Firstly, we need to check if the depot point is in file. Since, it can
    * be located at the end of the file and must be placed at the top, we must
    * process that point before handling the others.
    */
    char *line, *el;
    const char *depotstr = "//Depot: ";

    bool hasdepot = false;
    float temp;
    while (fgets(buf, LINE_SIZE, stream)){
        if (strncasecmp(buf, depotstr, strlen(depotstr)) != 0){
            continue;
        }
        line = &buf[strlen(depotstr)]; // just change pointer
        //first value X
        if ((el = strtok(line, ", ")) == NULL)
            continue;
        temp = strtod(el, NULL);
        set->point[0].x = temp;
        set->depot.x = temp;

        //second value Y
        if ((el = strtok(NULL, ", ")) == NULL)
            continue;

        temp = strtod(el, NULL);
        set->point[0].y = temp;
        set->depot.y = temp;

        set->point[0].z = 0;//the depot is on the ground
        set->depot.z = 0;

        //third value is radius for depot, usually it's 0 but...
        if ((el = strtok(NULL, ", ")) == NULL)
            continue;
        set->point[0].ra = strtod(el, NULL);
        set->point[0].rb = set->point[0].ra;

        set->depotidx = 0;
        set->size++;
        hasdepot = true;
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

        if (loadCETSPLine(&set->point[set->size], buf)){
            if (!hasdepot && set->size == 0){
                set->depotidx = 0;
                set->depot.x = set->point[0].x;
                set->depot.y = set->point[0].y;
                set->depot.z = set->point[0].z;
            }
            set->size++;
        }

    }

    fclose(stream);

    if(set->size <=0){
        fprintf(stderr, "Error while loading data from %s", filename);
        goto end;
    }

    set->point = realloc(set->point, sizeof(cetsp_point) * set->size);
end:
    return set;
}



/**
 * Free CETSP structure
 * @param cetsp_set CETSP structure
 */
void free_cetsp_set(cetsp_set *set){
    if (set == NULL){
        return;
    }

    free(set->point);
    set->point = NULL;

    free(set);
    set = NULL;
}


/**
 * Initialize CETSP solution structure and set all points to 0
 * @param size_t solution size;
 *
 * @return cetsp_set - CETSP solution structuree
 */
cetsp_solution * init_cetsp_sol(long size){
    if (size <= 0){
        fprintf(stderr, "set size invalide : %ld\n", (long)size);
        return NULL;
    }

    cetsp_solution *sol = malloc(sizeof(cetsp_solution));
    if (sol == NULL){
        perror("malloc sol");
        return NULL;
    }

    sol->size = size;

    sol->path = malloc(sizeof(unsigned) * size);
    if (sol->path == NULL){
        perror("malloc sol->path");
        free(sol);
        return NULL;
    }

    sol->tp = malloc(sizeof(eu_point) * size);
    if (sol->tp == NULL){
        perror("malloc sol->tp");
        free(sol);
        return NULL;
    }

    for(int i = 0; i < size; i++){
        sol->path[i] = i;
        sol->tp[i].x = 0.0;
        sol->tp[i].y = 0.0;
    }

    return sol;
}


/**
 * Initialize CETSP solution structure from file
 * @param const char *filename - file name
 * @param int limit - data limit size (no limit if <= 0 )
 *
 * @return cetsp_set - CETSP solution structuree
 */
cetsp_solution * load_cetsp_sol(const char *filename){
    char buf[LINE_SIZE];
    cetsp_solution *sol = NULL;

    FILE *stream = fopen(filename, "r");
    if (stream == NULL){
        warn("Error opening solution file %s", filename);
        return NULL;
    }

    int chr;
    size_t lines = 0;
    size_t pos = 0; //position of second line

    while ((chr = fgetc(stream)) != EOF){
        if (lines < 2){
            pos++;
        }
        if (chr == '\n'){
            lines++;
        }
    }

    //The file must have at least two lines
    if (lines <= 2) {
        goto end;
    }

    //init solution
    sol = init_cetsp_sol(lines - 2);

    //seek to 0 and 2 lines
    fseek(stream, pos, SEEK_SET);
    long line = -1;
    char *subtoken;

    while(fgets(buf, LINE_SIZE, stream)){
        line++;

        subtoken = strtok(buf, " ");
        if (subtoken == NULL){
            fprintf(stderr, "File format error\n");
            break;
        }
        int position = strtold(subtoken, NULL);
        if (position >= lines - 2){
            fprintf(stderr, "Wrong position\n");
            break;
        }
        sol->path[line] = position;

        subtoken = strtok(NULL, " ");
        if (subtoken == NULL){
            fprintf(stderr, "File format error\n");
            break;
        }
        sol->tp[line].x = strtod(subtoken, NULL);

        subtoken = strtok(NULL, " ");
        if (subtoken == NULL ){
            fprintf(stderr, "File format error\n");
            break;
        }
        sol->tp[line].y = strtod(subtoken, NULL);
    }

end:
    fclose(stream);
    return sol;
}


//Load and check helper structure
struct _pnts{
    double val;
    int index;
};
int _pnts_cmp(const void *a, const void *b) {
    return ( ((struct _pnts *)a)->val - ((struct _pnts *)b)->val);
}



/**
 * Free CETSP solution structure
 * @param cetsp_set CETSP solution structure
 */
void free_cetsp_sol(cetsp_solution *sol){
    if (sol == NULL)
        return;

    free(sol->path);
    sol->path = NULL;

    free(sol->tp);
    sol->tp = NULL;

    free(sol);
    sol = NULL;
}


distance_t cetsp_sol_eudist(cetsp_solution *sol){
    distance_t res = 0.0;
    for(long i=0; i < sol->size - 1 ; i++){
        res += hypot(sol->tp[i].x - sol->tp[i+1].x,
                     sol->tp[i].y - sol->tp[i+1].y);
    }
    res += hypot(sol->tp[sol->size - 1].x - sol->tp[0].x,
                 sol->tp[sol->size - 1].y - sol->tp[0].y);

    return res;
}


/**
 * Reduce points which have the depot in theirs neighborhood or dominated by another node
 *
 * @param CETSP structure
 * @return new size;
 */
long cetsp_cleanup(cetsp_set * set){
    //if depot is not defined
    if (set->depotidx < 0 ){
        return set->size;
    }

    long writeptr = 0;
    for(long i=0; i < set->size; i++){
        if (i == set->depotidx){
            memcpy(&set->point[writeptr++], &set->point[i], sizeof(cetsp_point));
            continue;
        }

        // if (hypot(set->point[set->depotidx].x - set->point[i].x, set->point[set->depotidx].y - set->point[i].y) > set->point[i].ra){
        //     memcpy(&set->point[writeptr++], &set->point[i], sizeof(cetsp_point));
        // }
        if (hypot(set->depot.x - set->point[i].x, set->depot.y - set->point[i].y) > set->point[i].ra){
            memcpy(&set->point[writeptr++], &set->point[i], sizeof(cetsp_point));
        }
    }
    set->size = writeptr;

    return set->size;
}



/**
 * Load and check sequence of a CETSP solution given in ile
 * @param seq - resulted sequence
 * @param set CETSP set
 * @param filename - file name
 * @return true if solution is feasible
 */
bool load_cetsp_sequence(permutation_t *seq, cetsp_set *set, const char *filename, double gap){
    bool result = false;
    int *visited = malloc(set->size * sizeof(int) );
    for(int i=0; i < set->size; i++){
        visited[i] = 0;
    }

    struct _pnts *p = malloc(2 * set->size * sizeof(struct _pnts));
    cetsp_solution *tsol = load_cetsp_sol(filename);
    if (tsol == NULL) goto end;
    double t1, t2;

    int riter = 0;
    for(int i=0; i < tsol->size; i++){
        int nbIntersections = 0;
        for (int j=0; j < set->size; j++){
            eu_point circle = {set->point[j].x, set->point[j].y, set->point[j].z};
            if (linecircle_intersection(&circle, set->point[j].ra + gap, &tsol->tp[i], &tsol->tp[(i+1) % tsol->size], &t1, &t2)){
                if (t1 >= 0 && t1 <= 1.0){
                    p[nbIntersections].index = j;
                    p[nbIntersections].val = t1;
                    nbIntersections++;
                }
                if (t2 >= 0 && t2 <= 1.0){
                    p[nbIntersections].index = j;
                    p[nbIntersections].val = t2;
                    nbIntersections ++;
                }
            }
        }

        qsort(p, nbIntersections, sizeof(struct _pnts), _pnts_cmp);
        // printf("Intersctions: ");
        // for(int k=0; k < nbIntersections; k++){
        //     printf("%d::%.2f  ", p[k].index, p[k].val);
        // }
        // putchar('\n');

        for (int j=0; j < nbIntersections; j++){
            if (visited[p[j].index] == 0){
                visited[p[j].index] = 1;
                seq[riter] = p[j].index;
                riter ++;
            }
        }
    }
    result = true;

    for(int i=0; i < set->size; i++){
        if (visited[i] == 0){
            printf("Solution loading failed");
            result = false;
            break;
        }
    }


end:
    free(p);
    free(visited);
    free_cetsp_sol(tsol);
    return result;
}

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
void cetsp_getgrid(cetsp_set *set, double *x, double *y, double *z){
    if (set == NULL){
        x = 0;
        y = 0;
        z = 0;
        return;
    }
    double xmax = -INFINITY;
    double xmin = INFINITY;
    double ymax = -INFINITY;
    double ymin = INFINITY;
    double zmax = -INFINITY;
    double zmin = INFINITY;

    for(long i=0; i < set->size; i++){
        if (set->point[i].x > xmax)
            xmax = set->point[i].x;
        if (set->point[i].x < xmin)
            xmin = set->point[i].x;
        if (set->point[i].y > ymax)
            ymax = set->point[i].y;
        if (set->point[i].y < ymin)
            ymin = set->point[i].y;
        if (set->point[i].x > zmax)
            zmax = set->point[i].z;
        if (set->point[i].z < zmin)
            zmin = set->point[i].z;
    }
    *x = xmax - xmin;
    *y = ymax - ymin;
    *z = zmax - zmin;
}

/**
 * Save CETSP instance to file
 * @param outfile - output filehandler
 * @param set  - CETSP instance
 * @return true if file was saved
 */
bool cetsp_save(FILE *outfh, cetsp_set *cset){
    if( outfh == NULL){
        return false;
    }

    fprintf(outfh, "//CETSP data set\n");
    fprintf(outfh, "//x y z ra rb\n");

    for(int i=0; i < cset->size; i++ ){
        if (i == cset->depotidx)
            continue;

        fprintf(outfh, "%.4f %.4f %.4f %.4f %.4f\n",
                cset->point[i].x,
                cset->point[i].y,
                cset->point[i].z,
                cset->point[i].ra,
                cset->point[i].rb
        );
    }

    //DEPOT point output
    if (cset->depotidx >= 0){
        fputs("\n", outfh);
        fprintf(outfh, "//Depot: %.4f, %.4f, %.4f\n",
            // cset->point[cset->depotidx].x,
            // cset->point[cset->depotidx].y,
            // cset->point[cset->depotidx].z
            cset->depot.x,
            cset->depot.y,
            cset->depot.z
        );
    }
    return true;
}

