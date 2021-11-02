/* 
TDT4258
Lab assignment 2 - Cache Simulation
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>

typedef enum {dm, fa} cache_map_t; //Direct mapped or Fully Associate
typedef enum {uc, sc} cache_org_t; //Unified or Split cache
typedef enum {instruction, data} access_t;

/* Here we define global variables */

const int b_offset = 6;
char type;
int fifo1;
int fifo2;
int n_blocks;
int branch;


typedef struct {
    uint32_t address;
    access_t accesstype;
} mem_access_t;

typedef struct {
    uint64_t accesses;
    uint64_t hits;
    // You can declare additional statistics if
    // you like, however you are now allowed to
    // remove the accesses or hits
} cache_stat_t;


/*Defining variables representing blocks and tag bits in cache. */
struct block1 {
    uint32_t tag;
};

struct block2{
    uint32_t tag2;
};

// DECLARE CACHES AND COUNTERS FOR THE STATS HERE
uint32_t cache_size; 
uint32_t block_size = 64;
cache_map_t cache_mapping;
cache_org_t cache_org;

//USE THIS FOR YOUR CACHE STATISTICS
cache_stat_t cache_statistics;


/* Reads a memory access from the trace file and returns
 * 1) access type (instruction or data access)
 * 2) memory address
 */
mem_access_t read_transaction(FILE *ptr_file) {
    mem_access_t access;
    if (fscanf(ptr_file, "%c %x\n", &type, &access.address) == 2) {
        if (type != 'I' && type != 'D') {
            printf("Unknown access type\n");
            exit(0);
        }
        access.accesstype = (type == 'I') ? instruction : data;
        return access;
    }
    /* If there are no more entries in the file,  
     * return an address 0 that will terminate the infinite loop in main
     */
    access.address = 0;
    return access;
}


void main(int argc, char** argv)
{

    // Reset statistics:
    memset(&cache_statistics, 0, sizeof(cache_stat_t));

    /* Read command-line parameters and initialize:
     * cache_size, cache_mapping and cache_org variables
     */

    if ( argc != 4 ) { /* argc should be 2 for correct execution */
        printf("Usage: ./cache_sim [cache size: 128-4096] [cache mapping: dm|fa] [cache organization: uc|sc]\n");
        exit(0);
    } else  {
        /* argv[0] is program name, parameters start with argv[1] */

        /* Set cache size */
        cache_size = atoi(argv[1]);

        /* Set Cache Mapping */
        if (strcmp(argv[2], "dm") == 0) {
            cache_mapping = dm;
        } else if (strcmp(argv[2], "fa") == 0) {
            cache_mapping = fa;
        } else {
            printf("Unknown cache mapping\n");
            exit(0);
        }

        /* Set Cache Organization */
        if (strcmp(argv[3], "uc") == 0) {
            cache_org = uc;
        } else if (strcmp(argv[3], "sc") == 0) {
            cache_org = sc;
        } else {
            printf("Unknown cache organization\n");
            exit(0);
        }
    }


    /* Open the file mem_trace.txt to read memory accesses */
    FILE *ptr_file;
    ptr_file =fopen("mem_trace.txt","r");
    if (!ptr_file) {
        printf("Unable to open the trace file\n");
        exit(1);
    }
    
    /* Splitting cache given chosen organization */

    if(cache_org == sc){
        n_blocks = (cache_size / block_size)/2;
        branch = n_blocks;       
    }

    else{
        n_blocks = (cache_size/ block_size);
        branch = n_blocks;
    }

    /* Defining pointers and malloc to allocate memory dynamically*/ 

    struct block1 *pnt = (struct block1*)malloc(sizeof(struct block1)*n_blocks);
    struct block2 *pnt2 = (struct block2*)malloc(sizeof(struct block2)*n_blocks);
    
    /* Loop until whole trace file has been read */
    mem_access_t access;
    while(1) {
        access = read_transaction(ptr_file);
        //If no transactions left, break out of loop
        if (access.address == 0)
            break;
	
    /* Do a cache access */

    //calculating set bits in integer
    uint32_t bits_set = log2(n_blocks);

    //When fully associate is set mapping, set bits = 0.
    if (cache_mapping == fa)
        bits_set = 0;


    //Getting the required tag bits from the address
    uint32_t index;
    uint32_t t_bits = 32 - b_offset - bits_set;
    uint32_t addressTag = (access.address >> (32 - t_bits));



    /* Setting index = 0 when number of blocks <= 2. 
    Otherwise, bit masking to get the needed index bits from the address. */
    if(n_blocks <= 2 && cache_org == sc){
        
        index = 0;
    }

    else{

        index = (access.address >> b_offset);
        index = (index << (b_offset + t_bits));
        index = (index >> (b_offset + t_bits));
    }
    //Initializing a hit tracker.
    int hit = 0;


    /* Using switch case to secure the correct method is called upon 
    Checking the pointers with malloc. Pnt for instruction and pnt2 for data. Increasing the counter if there is a hit.
    Addressing to tag if miss. */

    switch(cache_mapping){
        case dm:
            if (cache_org == uc){
                if ((pnt + index) -> tag == addressTag){ 
                    cache_statistics.hits++; 
                }
                else{
                    (pnt + index) -> tag = addressTag; 
                }
                break;                
            }
            else if(cache_org == sc){
                if(type == 'I'){
                    if ((pnt + index) -> tag == addressTag){
                        cache_statistics.hits++;
                    }
                    else{
                        (pnt + index) -> tag = addressTag;
                    }
                }
                else if(type == 'D'){
                    if ((pnt2 + index) -> tag2 == addressTag){
                        cache_statistics.hits++;
                    }
                    else{
                        (pnt2 + index) -> tag2 = addressTag;
                        }
                    }
                    break;
            }
        
        /* Looping through the branches to determine if hit or miss. Eject from address when neccesary 
        Using two pointers and two variables for fifo for functionality */
        case fa:
            if (cache_org == uc){
                for (int i = 0; i <= branch; i++){
                    if ((pnt + i) -> tag == addressTag){
                        cache_statistics.hits++;
                        hit = 1;
                        break;
                    }
                }

                if (hit == 0){
                    (pnt + fifo1) -> tag = addressTag;
                    fifo1++;
                    if (fifo1 == branch){
                        fifo1 = 0;
                    }
                }
                break;
            }
            
            else if(cache_org == sc){
                if(type == 'I'){
                    for(int i =0; i <= branch; i++){
                        if((pnt + i) -> tag == addressTag){
                            cache_statistics.hits++;
                            hit = 1;
                            break;
                        }
                    }
                    if(hit == 0){
                        (pnt + fifo1) -> tag = addressTag;
                        fifo1++;
                        if (fifo1 == branch){
                            fifo1 = 0;
                            }
                    }
                }
                if(type == 'D'){
                    for(int i = 0; i<= branch; i++){
                        if((pnt2 + i) -> tag2 == addressTag){
                            cache_statistics.hits++;
                            hit = 1;
                            break;
                        }
                    }
                    if(hit == 0){
                        (pnt2 + fifo2) -> tag2 = addressTag;
                        fifo2++;
                        if(fifo2 == branch){
                            fifo2 = 0;
                        }
                    }
                }
            }                                     
            break;
    }
         
    cache_statistics.accesses ++;
    
    }

    /* Print the statistics */
    // DO NOT CHANGE THE FOLLOWING LINES!
    printf("\nCache Statistics\n");
    printf("-----------------\n\n");
    printf("Accesses: %ld\n", cache_statistics.accesses);
    printf("Hits:     %ld\n", cache_statistics.hits);
    printf("Hit Rate: %.4f\n", (double) cache_statistics.hits / cache_statistics.accesses);
    // You can extend the memory statistic printing if you like!

    /* Close the trace file and clear allocated memory */
    free(pnt);
    free(pnt2);
    fclose(ptr_file);

}
