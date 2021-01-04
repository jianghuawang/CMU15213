#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "getopt.h"
#include "cachelab.h"

void run(int verbal,int bb,int E);
void intializeCache(int S,int E);
void freeCache(int S);
void parse(int argc,char* argv[],int *verbal,int *bb,int *sb,int* E);

typedef struct cacheLine{
    long long tag;
    int valid;
    int counter;
} CacheLine;

static CacheLine **cache;
static long long tgMask,stMask;
static int hits,misses,evictions;
static char fileName[20];

int main(int argc,char* argv[])
{
    int sb,bb,E,S,verbal=0;//sb means the number set bits, bb means the number of block bits, E means the line number, S means the set number
    parse(argc,argv,&verbal,&bb,&sb,&E);//parse the argument
    S=1<<sb;
    tgMask=-1<<(sb+bb);
    stMask=(-1<<(bb))^tgMask;
    intializeCache(S,E);
    run(verbal,bb,E);
    freeCache(S);
    printSummary(hits,misses,evictions);
    return 0;
}
void intializeCache(int S,int E){
    cache=(CacheLine**)malloc(S*sizeof(CacheLine*));
    for(int i=0;i<S;i++)
        cache[i]=(CacheLine*)calloc(E,sizeof(CacheLine));//we want each CachLine has its valid set to 0, so we call calloc here.
}
void freeCache(int S){
    for(int i=0;i<S;i++)
        free(cache[i]);
    free(cache);
}
void parse(int argc,char* argv[],int *verbal,int *bb,int *sb,int* E){
    int opt;
    while((opt=getopt(argc,argv,":hvs:E:b:t:"))!=-1){
        switch(opt){
            case 'h':
                printf("Usage: ./csim [-hv] -s <S> -E <E> -b <b> -t <tracefile>");
                break;
            case 'v':
                *verbal=1;
                break;
            case 's':
                *sb=atoi(optarg);
                break;
            case 'E':
                *E=atoi(optarg);
                break;
            case 'b':
                *bb=atoi(optarg);
                break;
            case 't':
                strcpy(fileName,optarg);
                break;
            case ':':
                fprintf(stderr,"option needs a value.\n");
                exit(1);
            case '?':
                fprintf(stderr,"unknown option: %c\n",optopt);
                exit(1);
        }
    }
}
void run(int verbal,int bb,int E){
    FILE *file=fopen(fileName,"r");
    char operation[2];
    int size;
    long long address,tag;
    unsigned long long stIndex;
    CacheLine* set;
    char hitString[]=" hit";
    char missString[]=" miss";
    char evictionString[]=" eviction";
    char oper;
    int time=0;
    while(fscanf(file,"%s %llx,%d\n",operation,&address,&size)!=EOF){
        ++time;
        oper= *operation;
        if(oper=='I')continue;
        stIndex=(address&stMask)>>bb;
        tag=address&tgMask;
        set=cache[stIndex];
        int lruIndex=0;
        int smallerCount=0x7fffffff;
        int hit=0;
        int miss=0;
        int eviction=0;
        for(int i=0;i<E;i++){
            if(set[i].valid&&set[i].tag==tag){
                hits+=(oper=='M')?2:1;
                set[i].counter=time;
                hit=1;
                break;
            }
            else{
                if(set[i].counter<smallerCount){
                    lruIndex=i;
                    smallerCount=set[i].counter;
                }
            }
        }
        if(!hit){
            misses+=(miss=1);
            hits+=(oper=='M')?1:0;
            if(set[lruIndex].valid)evictions+=(eviction=1);
            set[lruIndex].valid=1;
            set[lruIndex].tag=tag;
            set[lruIndex].counter=time;
        }
        if(verbal)
            printf("%c %llx,%i%s%s%s%s\n",oper,address,size,miss?missString:"",eviction?evictionString:"",hit?hitString:"",(oper=='M')?hitString:"");
    }
    fclose(file); 
}