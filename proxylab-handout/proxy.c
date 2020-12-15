#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "csapp.h"

/*
 * This is only a draft version which pass all tests.
 * Error handling and other details should still be improved(free memory, close fd, ...)
 * Also, lock granularity can be largely improved
 *
 */

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define ERROR_RET -1

/* You won't lose style points for including this long line in your code */
static char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static char *connection_hdr = "Connection: close\r\n";
static char *proxy_connection_hdr = "Proxy-Connection: close\r\n";
static char *empty_line = "\r\n";
static char *http_version = "HTTP/1.0";

/* Metadata node for cache */
struct cache_inode
{
  int size;
  char uri[MAXLINE];
  char *ptr;
  struct cache_inode *next;
  struct cache_inode *prev;
};

/* Head node of the cache */
struct cache_inode* head;
struct cache_inode* tail;

/* Global lock variable */
sem_t write_lock;

/* Total size of the cache, only count web data */
volatile ssize_t total_size;

struct cache_inode* get_node(char *path)
{
  struct cache_inode *node = head;
  while (node != tail) {
    if (node != head && !strcmp(path, node->uri)) {
      return node;
    }
    node = node->next;
  }

  return node;
}

void remove_last() {
  struct cache_inode* node = tail->prev;
  tail->prev->prev->next = tail;
  tail->prev = tail->prev->prev;
  free(node->ptr);
  free(node->uri);
  free(node);
}

void remove_from_cache(struct cache_inode* node)
{
  node->prev->next = node->next;
  node->next->prev = node->prev;
}

void add2cache(struct cache_inode* node)
{
  node->next = head->next;
  head->next->prev = node;
  node->prev = head;
  head->next = node;
}

/* Parse uri */
int parse_uri(char *uri, char *hostname, char *port, char *suffix)
{
  char host_info[MAXLINE];

  /* Parse hostname and port */ 
  sscanf(uri, "http://%[^/]%s", host_info, suffix);
  sscanf(host_info, "%[^:]:%s", hostname, port);
  if (!hostname) {
    return -1;
  }

  /* Use default port(80) if not specified */
  if (!port) {
    sprintf(port, "80");
  }

  return 0;
}

/* Read request header */
int make_requesthdrs(rio_t *client_rio, int server_fd, char *hostname, char *port)
{
  int has_host_hdr;
  char buf[MAXBUF], header_name[MAXLINE], header_content[MAXLINE], host_name_hdr[MAXLINE];

  Rio_readlineb(client_rio, buf, MAXLINE);
  while (strcmp(buf, empty_line)) {
    if (sscanf(buf, "%[^:]: %s", header_name, header_content) < 2) {
      return ERROR_RET;
    } 

    /* Get the host header */
    if (!strcasecmp(header_name, "Host")) {
      has_host_hdr = 1;
    } 

    /* Filter the user agent, connection and proxy-connection header */
    if (strcasecmp(header_name, "User-Agent") && strcasecmp(header_name, "Connection") && strcasecmp(header_name, "Proxy-Connection"))
      Rio_writen(server_fd, buf, strlen(buf));

    Rio_readlineb(client_rio, buf, MAXLINE);
  }

  /* Add user agent, connection and proxy-connection header */
  Rio_writen(server_fd, user_agent_hdr, strlen(user_agent_hdr));
  Rio_writen(server_fd, connection_hdr, strlen(connection_hdr));
  Rio_writen(server_fd, proxy_connection_hdr, strlen(proxy_connection_hdr));
  
  /* Add host header if not exist */
  if (!has_host_hdr) {
    sprintf(host_name_hdr, "Host: %s:%s\r\n", hostname, port);
    Rio_writen(server_fd, host_name_hdr, strlen(host_name_hdr));
  }

  return 0;
}

/* Read request line */
void make_request_line(int server_fd, char *method, char *suffix)
{
  char buf[MAXBUF];

  sprintf(buf, "%s %s %s\r\n", method, suffix, http_version);
  Rio_writen(server_fd, buf, strlen(buf));
}

/* Read from server*/
int handle_proxy(int server_fd, int client_fd, char *uri)
{
  char buf[MAXBUF];
  char *content = (char *)malloc(MAX_OBJECT_SIZE);
  ssize_t nsize, content_size = 0; 
  
  /* Get response from host and send back */

  nsize = 0;
  while ((nsize = Rio_readn(server_fd, buf, MAXBUF)) != 0) {
    if (content_size + nsize <= MAX_OBJECT_SIZE) {
      memcpy(content + content_size, buf, nsize); 
    }
    if (content_size <= MAX_OBJECT_SIZE) {
      content_size += nsize;
    }
    if (nsize < 0) {
      return ERROR_RET;
    }
    Rio_writen(client_fd, buf, nsize);
  } 

  if (content_size < MAX_OBJECT_SIZE) {
    P(&write_lock); 
    if (total_size + content_size > MAX_CACHE_SIZE) {
      remove_last();
    }
    
    struct cache_inode* node = (struct cache_inode*)malloc(sizeof(struct cache_inode));
    node->ptr = content; 
    node->size = content_size;
    strcpy(node->uri, uri);
    add2cache(node); 
    total_size += content_size;
    V(&write_lock);
  } else {
      /* Free content buf */
      free(content);
  }

  return 0;
}

void* handle(void *arg)
{
  Pthread_detach(Pthread_self());
  char client_buf[MAXLINE], hostname[MAXLINE], port[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], suffix[MAXLINE];
  struct cache_inode* node;
  rio_t client_rio;
  int server_fd;
  int client_fd;

  client_fd = *((int *)arg);

  /* Read request line and headers */
  Rio_readinitb(&client_rio, client_fd);
  Rio_readlineb(&client_rio, client_buf, MAXLINE);
  if (sscanf(client_buf, "%s %s %s", method, uri, version) < 3) {
    Close(client_fd);
    Pthread_exit(Pthread_self);
  }

  /* Check the cache with the uri */
  P(&write_lock);
  if ((node = get_node(uri)) != tail) {
    /* directly send the result */
    Rio_writen(client_fd, node->ptr, node->size);
    remove_from_cache(node);
    add2cache(node);
    V(&write_lock);
    Close(client_fd);
    Pthread_exit(Pthread_self);
  }
  V(&write_lock);

  /* Check http method */
  if (strcasecmp(method, "GET")) {
    Close(client_fd);
    Pthread_exit(Pthread_self);
  }
  
  /* Check http version */
  if (strcasecmp(version, "HTTP/1.0") && strcasecmp(version, "HTTP/1.1")) {
    Close(client_fd);
    Pthread_exit(Pthread_self);
  }

  /* Parse uri */
  if (parse_uri(uri, hostname, port, suffix) < 0) {
    Close(client_fd);
    Pthread_exit(Pthread_self);
  }

  /* Build connection to server */
  if ((server_fd = Open_clientfd(hostname, port)) < 0) {
    Close(client_fd);
    Pthread_exit(Pthread_self);
  }

  make_request_line(server_fd, method, suffix);

  /* Parse headers */
  if (make_requesthdrs(&client_rio, server_fd, hostname, port) < 0) {
    Close(server_fd);
    Close(client_fd);
    Pthread_exit(Pthread_self);
  }

  /* Add an empty line */
  Rio_writen(server_fd, empty_line, strlen(empty_line));
  
  /* Handle the proxy function */
  if (handle_proxy(server_fd, client_fd, uri) < 0) {
    Close(server_fd);
    Close(client_fd);
    Pthread_exit(Pthread_self);
  }

  Close(server_fd);
  Close(client_fd);

  return NULL;
}

int main(int argc, char **argv)
{
  int listenfd, connfd;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  pthread_t tid;

  /* Check command-line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  /* Initialize lock */
  Sem_init(&write_lock, 0, 1); 

  listenfd = Open_listenfd(argv[1]);

  head = (struct cache_inode*)malloc(sizeof(struct cache_inode));
  tail = (struct cache_inode*)malloc(sizeof(struct cache_inode));

  head->next = tail;
  tail->prev = head;
  total_size = 0;
  while(1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    void *connfd_ptr = malloc(sizeof(int));
    *(int *)connfd_ptr = connfd;
    /* Dispatch the connfd to a func(sequential) or a thread(concurrency) */
    Pthread_create(&tid, NULL, handle, connfd_ptr);
  }

  return 0;
}
