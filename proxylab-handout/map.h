
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define	MAXLINE	 8192
#define MAPSIZE 97

typedef struct cacheNode{
    char URL[MAXLINE];
    char *content;
    struct cacheNode *prev;
    struct cacheNode *next;
    int hash;
    int size;
} CacheNode;

typedef struct mapNode{
    char URL[MAXLINE];
    CacheNode *pos;
    struct mapNode *next;
} MapNode;

typedef struct {
    MapNode* m[MAPSIZE];
}HashMap;

int MurmurOAAT32(char *key);
void map_init(HashMap *hm);
void map_deinit(HashMap *hm);
void map_insert(HashMap *hm,char *URL,CacheNode* node);
CacheNode *map_find(HashMap *hm,char *URL);
void map_delete(HashMap *hm,char *URL);