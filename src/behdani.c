/*************************************\
 *  /\_/\    Clara GORBEAU     /\_/\  *
 * ( o.o )      @}}>---       ( o.o ) *
 *  > ^ <    (c) 2025-2026     > ^ <  *
\*************************************/
/**
 * Behdani's instances based generator
 *
 * [1] Behdani, Behnam, and J. Cole Smith. "An integer-programming-based approach to the close-enough traveling salesman problem."
 * INFORMS Journal on Computing 26.3 * (2014): 415-432.
 * DOI: https://pubsonline.informs.org/doi/suppl/10.1287/ijoc.2013.0574
 *
 */
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

#include "cetspwdc.h"
#include "config.h"

#define LINE_SIZE 256
#define PATH_SIZE 512

static struct stat st = {0};

int main(){
    int count = 0;
    char fullname[FILENAME_MAX];
    FILE *out;

    srand48(GENERATOR_SEED);
    printf("Processed %d files\n", count);

    //create directory if not exists
    if (stat(BEHDANI_WDC_DIR, &st) == -1)
        mkdir(BEHDANI_WDC_DIR, 0755);

    if (stat(PERFOMANCE_WDC_DIR, &st) == -1)
        mkdir(PERFOMANCE_WDC_DIR, 0755);

    const long sizes[] = {20, 30, 40, 50, 60, 70, 80, 90};
    const char classes[] = {'s', 'm', 'l'};
    const double rwrc[] = {0.1, 1.0}; //skip 0.5
    const double duration[] = {1.0, 5.0}; // skip 2.0

    const double grid = 16.0;
    const double baseradius = 1.0;
    const double class_low[] = {
        0.1, 0.4, 0.7
    };
    const double class_hight[] = {
        0.4, 0.7, 1.0
    };
    const double wakeuptime = 0.5;

    /**
     * 42 Parameter-testing instances
     * a) classes in {s, m, l}, rwrc = 0.5, duration = 2.0
     * b) classes=m, rwrc in {0.1, 0.5, 1.0}, duration = 2.0
     * c) classes=m, rwrc = 0.5, duration = {1.0, 2.0, 5.0}
     */

    for (long sz=0; sz< sizeof(sizes)/sizeof(long); sz++){

        wdc_set *wdcset = malloc(sizeof(wdc_set));
        if (wdcset == NULL){
            err(EXIT_FAILURE, "CETSPR: Out of memory");
        }
        wdcset->size = sizes[sz];
        wdcset->node = malloc(sizeof(wdc_node) * wdcset->size);

        //coordinate
        for (long i=0; i < wdcset->size; i++){
            wdcset->node[i].x = grid * drand48();
            wdcset->node[i].y = grid * drand48();
            wdcset->node[i].z = 0;
        }
        wdcset->depotidx = 0;
        wdcset->depot.x = wdcset->node[0].x;
        wdcset->depot.y = wdcset->node[0].y;
        wdcset->depot.z = wdcset->node[0].z;

        for(long class=0; class < 3; class++){

            //three classes radii
            for (long i=0; i < wdcset->size; i++){
                if (i==0){
                    wdcset->node[i].rc = 0;
                    wdcset->node[i].rw = 0;
                    wdcset->node[i].wakeuptime = 0;
                    wdcset->node[i].commtime = 0;
                }
                else{
                    wdcset->node[i].rc = baseradius * ((class_hight[class] - class_low[class]) * drand48() + class_low[class]);
                    wdcset->node[i].rw = 0.5 * wdcset->node[i].rc;
                    wdcset->node[i].wakeuptime = wakeuptime;
                    wdcset->node[i].commtime = wakeuptime*2;
                }
            }
            sprintf(fullname, "%s/%02ld-%c-05-2.cetspwdc",
                    BEHDANI_WDC_DIR,
                    sizes[sz],
                    classes[class]
            );
            printf("Generating %s\n", fullname);
            out = fopen(fullname, "w");
            wdc_save(out, wdcset);
            fclose(out);

            //middle class only
            if (class == 1){
                // b) classes=m, rwrc in {0.1, 1.0}, duration = wakeuptime * 2
                for (long j=0; j < sizeof(rwrc)/sizeof(double); j++){
                    sprintf(fullname, "%s/%02ld-m-%02d-2.cetspwdc",
                            BEHDANI_WDC_DIR,
                            sizes[sz],
                            (int)(rwrc[j]*10)
                    );
                    printf("Generating %s\n", fullname);
                    for (long n=0; n < wdcset->size; n++){
                        wdcset->node[n].rw = rwrc[j] * wdcset->node[n].rc;
                        wdcset->node[n].commtime = wakeuptime * 2;
                    }
                    out = fopen(fullname, "w");
                    wdc_save(out, wdcset);
                    fclose(out);
                }

                // c) classes=m, rwrc = 0.5, duration = {1.0, 5.0}
                for (long k=0; k < sizeof(duration)/sizeof(double); k++){
                    sprintf(fullname, "%s/%02ld-m-05-%d.cetspwdc",
                            BEHDANI_WDC_DIR,
                            sizes[sz],
                            (int)(duration[k])
                    );
                    printf("Generating %s\n", fullname);
                    for (long n=0; n < wdcset->size; n++){
                        wdcset->node[n].rw = 0.5 * wdcset->node[n].rc;;
                        wdcset->node[n].commtime = duration[k] * wakeuptime;
                    }
                    out = fopen(fullname, "w");
                    wdc_save(out, wdcset);
                    fclose(out);
                }
            }

        }
        free_wdc_set(wdcset);
    }

    //50 Instances for performance test
    const long sizesperf[] = {8, 10, 14, 16, 20};
    for (int k=0; k < 10;k++){
        for (long sz=0; sz< sizeof(sizesperf)/sizeof(long); sz++){

            wdc_set *wdcset = malloc(sizeof(wdc_set));
            if (wdcset == NULL){
                err(EXIT_FAILURE, "CETSPR: Out of memory");
            }
            wdcset->size = sizesperf[sz];
            wdcset->node = malloc(sizeof(wdc_node) * wdcset->size);

            //coordinate
            for (long i=0; i < wdcset->size; i++){
                wdcset->node[i].x = grid * drand48();
                wdcset->node[i].y = grid * drand48();
                wdcset->node[i].z = 0;
            }
            wdcset->depotidx = 0;
            wdcset->depot.x = wdcset->node[0].x;
            wdcset->depot.y = wdcset->node[0].y;
            wdcset->depot.z = wdcset->node[0].z;

            //three classes radii
            // for(long class=0; class < 3; class++){
                int class = 1;
                for (long i=0; i < wdcset->size; i++){
                    if (i==0){
                        wdcset->node[i].rc = 0;
                        wdcset->node[i].rw = 0;
                        wdcset->node[i].wakeuptime = 0;
                        wdcset->node[i].commtime = 0;
                    }
                    else{
                        wdcset->node[i].rc = baseradius * ((class_hight[class] - class_low[class]) * drand48() + class_low[class]);
                        wdcset->node[i].rw = 0.5 * wdcset->node[i].rc;
                        wdcset->node[i].wakeuptime = wakeuptime;
                        wdcset->node[i].commtime = wakeuptime*2;
                    }
                }
                sprintf(fullname, "%s/%02ld-%c-05-2.%d.cetspwdc",
                        PERFOMANCE_WDC_DIR,
                        sizesperf[sz],
                        classes[class],
                        k
                );
                printf("Generating(perf) %s\n", fullname);
                out = fopen(fullname, "w");
                wdc_save(out, wdcset);
                fclose(out);
            // }

            free_wdc_set(wdcset);
        }
    }

    puts("***Finished***");
    return EXIT_SUCCESS;
}

