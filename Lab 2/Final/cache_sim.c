#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef enum { dm, fa } cache_map_t;
typedef enum { uc, sc } cache_org_t;
typedef enum { instruction, data } access_t;


// Global variables
const int b_offset = 6;

int fifo1;
int fifo2;
int num_blocks;
int branch;

char type;

uint32_t bits_set;


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

/*Defining variables representing blocks and tag bits in cache. Used for split cache*/
typedef struct block1 {
    uint32_t tag;
} block1;
block1 *pnt;

typedef struct block2 {
    uint32_t tag2;
} block2;
block2 *pnt2;

// DECLARE CACHES AND COUNTERS FOR THE STATS HERE

uint32_t cache_size;
uint32_t block_size = 64;
cache_map_t cache_mapping;
cache_org_t cache_org;

// USE THIS FOR YOUR CACHE STATISTICS
cache_stat_t cache_statistics;

/* Reads a memory access from the trace file and returns
 * 1) access type (instruction or data access
 * 2) memory address
 */
mem_access_t read_transaction(FILE* ptr_file) {
  mem_access_t access;

  if (fscanf(ptr_file, "%c %x\n", &type, &access.address) == 2) {
    if (type != 'I' && type != 'D') {
      printf("Unkown access type\n");
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

void main(int argc, char** argv) {
  // Reset statistics:
  memset(&cache_statistics, 0, sizeof(cache_stat_t));

  /* Read command-line parameters and initialize:
   * cache_size, cache_mapping and cache_org variables
   */
  /* IMPORTANT: *IF* YOU ADD COMMAND LINE PARAMETERS (you really don't need to),
   * MAKE SURE TO ADD THEM IN THE END AND CHOOSE SENSIBLE DEFAULTS SUCH THAT WE
   * CAN RUN THE RESULTING BINARY WITHOUT HAVING TO SUPPLY MORE PARAMETERS THAN
   * SPECIFIED IN THE UNMODIFIED FILE (cache_size, cache_mapping and cache_org)
   */
  if (argc != 4) { /* argc should be 2 for correct execution */
    printf(
        "Usage: ./cache_sim [cache size: 128-4096] [cache mapping: dm|fa] "
        "[cache organization: uc|sc]\n");
    exit(0);
  } else {
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
  FILE* ptr_file;
  ptr_file = fopen("mem_trace.txt", "r");
  if (!ptr_file) {
    printf("Unable to open the trace file\n");
    exit(1);
  }


  /* Loop until whole trace file has been read */



  /* Splitting cache given chosen organization */
 // Number of blocks and index bits are calculated
 if(cache_org == sc){
      num_blocks = (cache_size / block_size)/2; 
      branch = num_blocks;                      
      bits_set = log2(num_blocks);     
  }

  // Setting bits to 0 when fully associated is used
  else if((cache_mapping == fa) && (cache_org == uc)){
    num_blocks = (cache_size/ block_size);
    branch = num_blocks;
    bits_set = 0;
  }

   else if((cache_mapping == fa) && (cache_org == sc)){
    num_blocks = (cache_size/ block_size);
    branch = num_blocks;
    bits_set = 0;
  }

  else{
      num_blocks = (cache_size/ block_size);
      branch = num_blocks;
      bits_set = log2(num_blocks);
  }
 
  // Defining index. Number of tag bit is calculated
  uint32_t index;
  uint32_t t_bits = 32 - b_offset - bits_set;
  
  /* Defining pointers and calling to malloc to allocate memory dynamically*/ 
  pnt = (block1*)malloc(sizeof(block1) * num_blocks);
  pnt2 = (block2*)malloc(sizeof(block2) * num_blocks);


  mem_access_t access;
  while (1) {
    access = read_transaction(ptr_file);
    // If no transactions left, break out of loop
    if (access.address == 0) break;
    //printf("%d %x\n", access.accesstype, access.address); 
    /* Do a cache access */
    // ADD YOUR CODE HERE

    //Initializing a hit tracker.
    int hit = 0;

    /* Using switch case to secure the correct method is called upon 
    Checking the pointers with malloc. Pnt for instruction and pnt2 for data. Increasing the counter if there is a hit.
    Addressing to tag if miss. */

    switch(cache_mapping){
      //Direct mapped, unified cache
        case dm:
            if (cache_org == uc){
                index = (access.address >> b_offset);                              // Bit masking to get the index bits from address read
                index = (index << (b_offset - t_bits));
                index = (index >> (b_offset + t_bits));
                if ((pnt + index) -> tag == (access.address >> (32 - t_bits))){    // Using pointer allocated ptr to determine hit/miss
                    cache_statistics.hits++; 
                }
                else{
                    (pnt + index) -> tag = (access.address >> (32 - t_bits));      // Assign address to tag if miss
                }
                break;                
            }
            // Direct mapped, split cache
            else if(cache_org == sc){
                if(type == 'I'){                                                   // Using two pointers, ptr for instruction and ptr2 for data
                    if ((pnt + index) -> tag == (access.address >> (32 - t_bits))){
                        cache_statistics.hits++;
                    }
                    else{
                        (pnt + index) -> tag = (access.address >> (32 - t_bits));
                    }
                }
                else if(type == 'D'){
                    if ((pnt2 + index) -> tag2 == (access.address >> (32 - t_bits))){
                        cache_statistics.hits++;
                    }
                    else{
                        (pnt2 + index) -> tag2 = (access.address >> (32 - t_bits));
                        }
                    }
                    break;
            }
        // Fully associative, unified cache
        case fa:                                                          
            if (cache_org == uc){                                                 
                for (int i = 0; i <= branch; i++){                                // Looping through the branches while checking for hit/miss
                    if ((pnt + i) -> tag == (access.address >> (32 - t_bits))){
                        cache_statistics.hits++;
                        hit = 1;
                        break;
                    }
                }

                if (hit == 0){                                                    // Ejecting first address from cache if branches are filled
                    (pnt + fifo1) -> tag = (access.address >> (32 - t_bits));
                    fifo1++;
                    if (fifo1 == branch){
                        fifo1 = 0;
                    }
                }
                break;
            }
            // Fully associative, split cache
            else if(cache_org == sc){
                if(type == 'I'){                                                    // Repeating the instructions with two fifo variables
                    for(int i =0; i <= branch; i++){
                        if((pnt + i) -> tag == (access.address >> (32 - t_bits))){
                            cache_statistics.hits++;
                            hit = 1;
                            break;
                        }
                    }
                    if(hit == 0){
                        (pnt + fifo1) -> tag = (access.address >> (32 - t_bits));
                        fifo1++;
                        if (fifo1 == branch){
                            fifo1 = 0;
                            }
                    }
                }
                if(type == 'D'){
                    for(int i = 0; i<= branch; i++){
                        if((pnt2 + i) -> tag2 == (access.address >> (32 - t_bits))){
                            cache_statistics.hits++;
                            hit = 1;
                            break;
                        }
                    }
                    if(hit == 0){
                        (pnt2 + fifo2) -> tag2 = (access.address >> (32 - t_bits));
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
  printf("Hit Rate: %.4f\n",
         (double)cache_statistics.hits / cache_statistics.accesses);
  // DO NOT CHANGE UNTIL HERE
  // You can extend the memory statistic printing if you like!

  /* Close the trace file */
  free(pnt);
  free(pnt2);
  fclose(ptr_file);
}
