#include<stdlib>

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define	MAXLINE	 8192
#define MAPSIZE 997

typedef uint32_t int;

typedef struct {
    char content[MAX_OBJECT_SIZE];
    char URL[MAXLINE];
    Node* next;
    Node* prev;
    int size;
} Node;

typedef struct {
    char URL[MAXLINE];
    Node *pos;
    MapNode *next;
} MapNode;

typedef struct {
    Node *head;
    Node *tail;
} LinkedList;

typedef struct {
    MapNode* map[MAPSIZE];
    LinkedList lst;
    int count;
    sem_t mutex;
    sem_t w;
} LRU;

uint32_t inline MurmurOAAT32(const char *key);

void LRU_init(LRU *lru);
void insert(LRU *lru,char *URL,char *content,int size);
char *search(LRU *lru,char *URL,int *size);
static void remove(Node *n);
static Node *get_node(LRU *lru,char *URL);