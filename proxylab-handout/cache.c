#include "cache.h"
#include "csapp.h"

uint32_t inline MurmurOAAT32(const char * key)
{
    uint32_t h(3323198485ul);
    for (;*key;++key) {
        h ^= *key;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

HashMap *construct_map(){
    HashMap* hm=malloc(sizeof(HashMap));
    int i=0;
    for(;i<MAPSIZE;i++){
        hm->map[i].head=NULL;
        hm->map[i].tail=NULL;
    }
    hm->count=0;
    sem_init(&hm->mutex,0,1);
    sem_init(&hm->w,0,1);
    return hm;
}
void destruct_map(HashMap* hm){
    free(hm);
}

void insert_item()

