#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <assert.h>
#include "cachelab.h"
#include <math.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#define DEBUG_ON
#define ADDRESS_LENGTH 64

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
    mem_addr_t temp = ~(0x0);
    set_index_mask = (temp >> (ADDRESS_LENGTH-s-b)) & (temp << b);//only set field are 1
    cache = malloc(S*sizeof(cache_set_t));
    for (i=0; i<S; i++){
        cache[i] = malloc(E*sizeof(cache_line_t));
        for(j=0; j<E; j++){
            cache[i][j].valid = '0';
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

void accessData(mem_addr_t addr){
    int i;
    //int index;  
    mem_addr_t addr_tag = addr >> (s+b);
    mem_addr_t addr_set = (addr & set_index_mask) >> b;
    //mem_addr_t addr_set = addr << (ADDRESS_LENGTH-s-b);
    //addr_set = addr_set >> (ADDRESS_LENGTH-s);
    //unsigned long long int templru = cache[addr_set][0].lru;
    cache_set_t evict;
    evict = &cache[addr_set][0];

    for(i=0; i<E; i++){     
       if(cache[addr_set][i].tag == addr_tag && cache[addr_set][i].valid != '0'){
            hit_count++;//hit
            lru_counter++;
            cache[addr_set][i].lru = lru_counter;
            if(verbosity) printf(" hit ");
            return;
        }else if(cache[addr_set][i].valid == '0'){
            miss_count++;//miss
            cache[addr_set][i].valid = '1';
            cache[addr_set][i].tag = addr_tag;
            lru_counter++;
            cache[addr_set][i].lru = lru_counter;
            if(verbosity) printf(" miss ");
            return;
       }else{
           // if(cache[addr_set][i].lru < templru){ 
               // templru = cache[addr_set][i].lru;
               // index = i;
           // }
            if(cache[addr_set][i].lru < evict->lru)
                    evict = &cache[addr_set][i];
       }

    }
    miss_count++;
    if(verbosity) printf(" miss ");
    eviction_count++;//eviction
   // cache[addr_set][index].tag = addr_tag;
    evict->tag = addr_tag;
   // cache[addr_set][index].valid = '1';
    evict->valid = '1';
    lru_counter++;
   // cache[addr_set][index].lru = lru_counter;
    evict->lru = lru_counter;
    if(verbosity) printf(" eviction ");
    return;
}

void replayTrace(char* trace_fn){
   char buf[1000];
   mem_addr_t addr = 0;
   unsigned int len = 0;
   FILE* trace_fp = fopen(trace_fn,"r");

   if(!trace_fp){
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
   }

   while(fgets(buf,1000,trace_fp) != NULL){
        if(buf[1]=='S' || buf[1]=='L' || buf[1]=='M') {
            sscanf(buf+3,"%llx,%u",&addr,&len);

            if(verbosity)
                printf("%c %llx,%u",buf[1],addr,len);

            accessData(addr);

            if(buf[1]=='M')
                accessData(addr);

            if(verbosity)
                printf("\n");
        }
   }

   fclose(trace_fp);
}

void printUsage(char* argv[]){
   
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



int main(int argc,char* argv[])
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

#ifdef DEBUG_ON
    printf("DEBUG: S:%u E:%u B:%u trace:%s\n",S,E,B,trace_file);
    printf("DEBUG: set_index_mask: %llu\n",set_index_mask);
#endif

    replayTrace(trace_file);

    freeCache();

    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
