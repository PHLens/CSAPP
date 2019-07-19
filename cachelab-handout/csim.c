#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <assert.h>
#include "cachelab.h"
#include <math.h>


typedef unsigned long long int mem_addr_t;

typedef struct cache_line {
    char valid;
    mem_addr_t tag;
    unsigned long long int lru;
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;

int verbosity = 0;
int s = 0;/*bits of set field*/
int b = 0;/*bits of block field*/
int E = 0;
char* trace_file = NULL;

int S;/*number of sets*/
int B;/*size of block*/

int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;
unsigned long long int lru_counter = 1;

/*The cache simulator*/
cache_t cache;
mem_addr_t set_index_mask;

void initCache(){
    int i,j;
    cache = malloc(S*sizeof(cache_set_t));
    for (i=0; i<S; i++){
        cache[i] = malloc(E*sizeof(cache_line_t));
        for(j=0; j<E; j++){
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].lru = 1;
        }
    }
}

void freeCache(){
    for(int i=0; i<S; i++){
       free(cache[i]);
    }
    free(cache);
}

void accessData(mem_add_t addr){
    unsigned long long int temp = addr;
    unsigned long long int addr_tag = temp >> (s+b);
    unsigned long long int addr_set = temp << (ADDRESS_LENGTH-(s+b));
    addr_set = addr_set >> (ADDRESS_LENGTH-s);
    int i;

    for(i=0; i<E; i++){
       if(cache[addr_set][i].tag == addr_tag && cache[addr_set][i].valid != 0){
            hit_count++;//hit
            lru_counter++;
            cache[addr_set][i].lru = lru_counter;
            break;
        }

    }

}

void replayTrace(char* trace_fn){

}

void printUsage(char* arg[]){
   
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}



int main()
{
    char opt;
   
    while((opt=getopt(argc,argv,"s:E:b:t:vh")) != -1){
        switch(opt){
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
            case 'v':
                verbosity = 1;
                break;
            case 'h':
                printUsage(argv);
                exit(0);
            default:
                printUsage(argv);
                exit(1);
        }
    }

    if(s == 0 || E == 0 || b == 0 || trace_file == NULL){
        printf("%s:Missing required command line argument\n",argv[0]);
        printUsage(argv);
        exit(1);
    }

    /*compute S, E and B from command line args*/
    S = pow(2,s);
    B = pow(2,b);
    
    /*Initialize cacbe*/
    initCache();

    replayTrace(trace_file);

    freeCache();
    printSummary(0, 0, 0);
    return 0;
}
