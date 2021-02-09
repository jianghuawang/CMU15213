#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define NTHREAD 8
#define SBUFSIZE 16

typedef struct {
    int *buf;
    int n;
    int front;
    int rear;
    sem_t mutex;
    sem_t slots;
    sem_t items;
} sbuf_t;

void doit(int fd);
void init_headers(char *header);
int read_requesthdrs(rio_t *rp,char *header);
int parse_url(char *url,char *hostname,char *reqeust_line,char *port);
void send_request(int serverfd,char *request_line,char *header);
void redirect_response(int clientfd,rio_t *rio_server,char *buf);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);

void sbuf_init(sbuf_t *sp,int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp,int item);
int sbuf_remove(sbuf_t *sp);
void *thread(void *vargp);


void sbuf_init(sbuf_t *sp,int n){
    sp->buf = Calloc(n,sizeof(int));
    sp->n=n;
    sp->front=sp->rear=0;
    sem_init(&sp->mutex,0,1);
    sem_init(&sp->slots,0,n);
    sem_init(&sp->items,0,0);
}

void sbuf_deinit(sbuf_t *sp){
    free(sp->buf);
}
void sbuf_insert(sbuf_t *sp,int item){
    P(&sp->slots);
    P(&sp->mutex);
    sp->buf[(++sp->rear)%(sp->n)]=item;
    V(&sp->mutex);
    V(&sp->items);
}
int sbuf_remove(sbuf_t *sp){
    int item;
    P(&sp->items);
    P(&sp->mutex);
    item=sp->buf[(++sp->front)%(sp->n)];
    V(&sp->mutex);
    V(&sp->slots);
    return item;
}
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static sbuf_t sbuf;

int main(int argc,char* argv[])
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    /* Check the command line args*/
    if(argc != 2){
        fprintf(stderr,"usage: %s <port>\n",argv[0]);
        exit(1);
    }
    listenfd = Open_listenfd(argv[1]);
    
    sbuf_init(&sbuf,SBUFSIZE);

    for(int i=0;i<NTHREAD;i++)
        pthread_create(&tid,NULL,thread,NULL);

    while(1){
        clientlen = sizeof(clientaddr);
        connfd=Accept(listenfd,(SA *)&clientaddr,&clientlen);
        printf("connect to client on file descriptor %d\n",connfd);
        sbuf_insert(&sbuf,connfd);
    }
    sbuf_deinit(&sbuf);
    return 1;
}

void *thread(void *vargp){
    pthread_detach(pthread_self());
    while(1){
        int connfd = sbuf_remove(&sbuf);
        doit(connfd);
        close(connfd);
    }
}

void doit(int fd){
    char buf[MAXLINE],method[MAXLINE],url[MAXLINE],version[MAXLINE];
    char header[MAXLINE]="",hostname[MAXLINE];
    char port[MAXLINE]="80",request_line[MAXLINE]="GET ";
    int has_host,serverfd;
    rio_t rio_client,rio_server;

    /* Read request line */
    rio_readinitb(&rio_client,fd);
    if(!rio_readlineb(&rio_client,buf,MAXLINE))
        return;
    /* divide the request line into request method, URL and HTTP version */
    if(sscanf(buf,"%s %s %s",method,url,version)!=3)
        return;
    /* Check if the request is a GET request*/
    if(strcasecmp(method,"GET")){
        clienterror(fd,method,"501","Not Implemented",
                    "the proxy does not implement this method");
        return;
    }

    /* Parse the URL into hostname and uri and even the port */
    if(!parse_url(url,hostname,request_line,port)){
        clienterror(fd,method,"400","Bad request","Hostname is invalid");
        return;
    }

    strcat(request_line," HTTP/1.0\r\n");

    //initialize the header
    init_headers(header);

    //fill the header
    has_host=read_requesthdrs(&rio_client,header);

    if(has_host==-1){
        clienterror(fd,method,"400","Bad request","Request header is invalid");
        return;
    }
    else if(has_host==0)
        sprintf(header,"%sHost: %s\r\n\r\n",header,hostname);
    else
        strcat(header,"\r\n");
    
    serverfd=open_clientfd(hostname,port);

    //cannot build connection 
    if(serverfd<0){
        clienterror(fd,method,"400","Bad request","Request is invalid");
        return;
    }

    rio_readinitb(&rio_server,serverfd);
    
    send_request(serverfd,request_line,header);

    redirect_response(fd,&rio_server,buf);

    close(serverfd);
}

void init_headers(char *header){
    strcat(header,user_agent_hdr);
    strcat(header,"Connection: close\r\n");
    strcat(header,"Proxy-Connection: close\r\n");
}

int read_requesthdrs(rio_t *rp,char *header){
    char *pos,buf[MAXLINE];
    size_t size;
    int has_host=0;
    size=Rio_readlineb(rp,buf,MAXLINE);
    while(strcmp(buf,"\r\n")){
        pos=strchr(buf,':');
        if(!pos)
            return -1;
        *pos='\0';
        if(!strcmp(buf,"Host"))
            has_host=1;
        else if(!strcmp(buf,"Proxy-Connection")||!strcmp(buf,"Connection")||!strcmp(buf,"User-Agent")){
            size=Rio_readlineb(rp,buf,MAXLINE);
            continue;
        }
        *pos=':';
        strncat(header,buf,size);
        size=Rio_readlineb(rp,buf,MAXLINE);
    }
    return has_host;
}

int parse_url(char *url,char *hostname,char *reqeust_line,char *port){
    char *colon_pos, *slash_pos, *host_begin;
    host_begin=strstr(url,"http://");
    if(!host_begin)return 0;
    host_begin+=7;
    colon_pos=strchr(host_begin,':');
    if(colon_pos){
        *colon_pos='\0';
        strcpy(hostname,host_begin);
        slash_pos=strchr(colon_pos+1,'/');
        if(!slash_pos){
            strcpy(port,colon_pos+1);
            strcat(reqeust_line,"/");
            return 1;
        }
        *slash_pos='\0';
        strcpy(port,colon_pos+1);
        *slash_pos='/';
    }
    else{
        slash_pos=strchr(host_begin,'/');
        if(!slash_pos){
            strcpy(hostname,host_begin);
            strcat(reqeust_line,"/");
            return 1;
        }
        *slash_pos='\0';
        strcpy(hostname,host_begin);
        *slash_pos='/';
    }
    strcat(reqeust_line,slash_pos);
    return 1;
}
void send_request(int serverfd,char *request_line,char *header){
    rio_writen(serverfd,request_line,strlen(request_line));
    rio_writen(serverfd,header,strlen(header));
    printf("%s",request_line);
    printf("%s",header);
}

void redirect_response(int clientfd,rio_t *rio_server,char *buf){
    size_t size;
    while((size=rio_readnb(rio_server,buf,MAXLINE))!=0){
        rio_writen(clientfd,buf,size);
    }
}

void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Proxy Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Proxylab Web Proxy</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
