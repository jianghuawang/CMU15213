#include<stdlib>

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define	MAXLINE	 8192
#define MAPSIZE 997

typedef struct{
    Node* head;
    Node* tail; 
} LinkedList;

typedef struct {
    char URL[MAXLINE];
    char content[MAX_OBJECT_SIZE];
    Node* prev;
    Node* next;
} Node;

typedef struct {
    LinkedList map[MAPSIZE];
    int count;
    sem_t mutex;
    sem_t w;
} HashMap;

uint32_t inline MurmurOAAT32(const char *key);
HashMap *construct_map();
void destruct_map(HashMap* hm);


