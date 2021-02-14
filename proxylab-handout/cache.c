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

void LRU_init(LRU *lru){
    int i;
    for(i=0;i<MAPSIZE;i++)
        lru->map[i]=NULL;
    lru->lst.head=NULL;
    lru->lst.tail=NULL;
    lru->count=0;
    sem_init(&lru->mutex,0,1);
    sem_init(&lru->w,0,1);
}

void insert(LRU *lru,char *URL,char *content,int size){
    Node * curr=get_node(lru,URL);
    if(curr){
        strcat(curr->content,content);
    }
    else{
        P(&lru->w);
        curr=malloc(sizeof(Node));
        curr->next=lru->lst.head;
        curr->prev=NULL;
        if(lru->lst.head)lru->lst.head->prev=curr;
        if(!lru->lst.tail)lru->lst.tail=curr;
        lru->head=curr;
        map_insert(lru,url,curr);
    }
}