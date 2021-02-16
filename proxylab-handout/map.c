#include "map.h"
#include "csapp.h"


int MurmurOAAT32(char * key)
{
    int h=3323198485ul;
    for (;*key;++key) {
        h ^= *key;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}
void map_init(HashMap *hm){
    for(int i=0;i<MAPSIZE;i++)
        hm->m[i]=NULL;
}
void map_deinit(HashMap *hm){
    for(int i=0;i<MAPSIZE;i++){
        MapNode *curr=hm->m[i];
        while(curr){
            MapNode *temp=curr->next;
            free(curr);
            curr=temp;
        }
    }
}
void map_insert(HashMap *hm,char *URL,CacheNode* node){
    int hash=MurmurOAAT32(URL)%MAPSIZE;
    MapNode *curr=(MapNode *)malloc(sizeof(MapNode));
    curr->pos=node;
    strcpy(curr->URL,URL);
    curr->next=hm->m[hash];
    hm->m[hash]=curr;
}
CacheNode *map_find(HashMap *hm,char *URL){
    int hash=MurmurOAAT32(URL)%MAPSIZE;
    MapNode *curr=hm->m[hash];
    while(curr){
        if(!strcmp(URL,curr->URL)){
            return curr->pos;
        }
        curr=curr->next;
    }
    return NULL;
}
void map_delete(HashMap *hm,char *URL){
    int hash=MurmurOAAT32(URL)%MAPSIZE;
    MapNode *curr=hm->m[hash];
    MapNode *prev=NULL;
    while(curr){
        if(!strcmp(URL,curr->URL)){
            if(prev)prev->next=curr->next;
            else hm->m[hash]=curr->next;
            free(curr);
            return;
        }
        prev=curr;
        curr=curr->next;
    }
}