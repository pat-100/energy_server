// SPDX-License-Identifier:  GPL-3.0-or-later

#include "httpd.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CONNECTIONS 3

static int *clients;
static struct header_s reqhdr[32] = {{"\0", "\0"}};
static int listenfd;
static int start_server(const char *PORT);
static int respond(int n);

int serve_forever(const char *PORT)
{
  struct sockaddr_in clientaddr;
  socklen_t addrlen;
  char c;
  int i, slot = 0;

  /*fprintf(stderr, "Server started %shttp://127.0.0.1:%s%s\n", "\033[92m", to_string(PORT),
      "\033[0m");*/
  
  // create shared memory for client slot array
  clients = (int*)malloc(sizeof(*clients) * MAX_CONNECTIONS);
  /*clients = mmap(NULL, sizeof(*clients) * MAX_CONNECTIONS,
      PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);*/
  
  // Setting all elements to -1: signifies there is no client connected
  for (i = 0; i < MAX_CONNECTIONS; i++) {
    clients[i] = -1;
  }
  start_server(PORT);
  
  // ACCEPT connections
  addrlen = sizeof(clientaddr);

  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  while (1) {
    clients[slot] = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);
    if (setsockopt(clients[slot], SOL_SOCKET, SO_RCVTIMEO, &timeout,
                                                      sizeof(timeout)) < 0) {
      perror("set timeout failed\n");
      return -1;
    }
    
    if (clients[slot] < 0) {
      perror("accept() error");
      exit(1);
    } else {
		//pthread_mutex_init(&(threads[i].data_lock), NULL);
		pthread_t thread_id;
		int* p_slot = (int*)malloc(sizeof(int));
		*p_slot = slot;
		pthread_create(&thread_id, NULL, &handle_client, (void*) p_slot);
 
      /*if (fork() == 0) {
        close(listenfd);
        if (respond(slot) != 0)
          fprintf(stderr, "ERROR: Failed to respond to request\n");
        // send settings to pipe
          exit(0);
      } else {
        wait(NULL);
        close(clients[slot]);
        clients[slot] = -1;
      }*/
    }
    
    while (clients[slot] != -1)
      slot = (slot + 1) % MAX_CONNECTIONS;
  }

  // Should never return, if it does then an error occured
  return -1;
}

void* handle_client(void* p_slot){
	int slot = *((int*)p_slot);
	free(p_slot);
	//close(listenfd);
    if (respond(slot) != 0)fprintf(stderr, "ERROR: Failed to respond to request\n");
	close(clients[slot]);
    clients[slot] = -1;
	return NULL;
}

// start server
static int start_server(const char *port)
{
  struct addrinfo hints, *res, *p;
  
  // getaddrinfo for host
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if (getaddrinfo(NULL, port, &hints, &res) != 0) {
    perror("getaddrinfo() error");
    return -1;
  }
  // socket and bind
  for (p = res; p != NULL; p = p->ai_next) {
    int option = 1;
    listenfd = socket(p->ai_family, p->ai_socktype, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option));
    if (listenfd == -1)
      continue;
    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
      break;
  }
  if (p == NULL) {
    perror("socket() or bind()");
    return -1;
  }
  
  freeaddrinfo(res);
  
  // listen for incoming connections
  if (listen(listenfd, MAX_CONNECTIONS) != 0) {
    perror("listen() error");
    return -1;
  }

  return 0;
}

// get request header by name
char *request_header(const char *name)
{
  struct header_s *h = reqhdr;
  while (h->name) {
    if (strcasecmp(h->name, name) == 0)
      return h->value;
    h++;
  }
  return NULL;
}

// get all request headers
struct header_s *request_headers(void)
{
  return reqhdr;
}

struct file_fifo_s *file_push(struct file_fifo_s *fifo, char *name, size_t size)
{
  struct file_fifo_s *ret, *tmp;
  struct file_fifo_s *new_ = (struct file_fifo_s *)malloc(sizeof(struct file_fifo_s));
  new_->fname = strdup(name);
  new_->offset = 0;
  new_->size = size;
  new_->mode[0] = 'w';
  new_->mode[1] = '\0';
  new_->next = NULL;

  tmp = fifo;
  if (!fifo) {
    ret = new_;
  } else {
    while (tmp->next != NULL) {
      tmp = tmp->next;
    }
    tmp->next = new_;
    ret = fifo;
  }

  return ret;
}

struct file_fifo_s *file_pop(struct file_fifo_s *fifo)
{
  if (fifo == NULL)
    return NULL;

  struct file_fifo_s *t = fifo->next;
  if (!t)
    return NULL;

  if (!fifo->fname)
    return NULL;
  free(fifo->fname);
  free(fifo);
  return t;
}

// client connection
static int respond(int n)
{
  int rcvd, fd, bytes_read, is_req_valid;
  char *ptr;
  char *buf, *tmp;
  struct http_req_s http_req = { 0 };
  int buf_size = 65536;
  int clientfd;
  char *uri;

  buf = (char*)malloc(buf_size);
  rcvd = recv(clients[n], buf, buf_size, 0);

  if (rcvd < 0) { // receive error
    fprintf(stderr, ("recv() error\n"));
    free(buf);
    return -1;
  } else if (rcvd == 0) { // receive socket closed
    fprintf(stderr, "Client disconnected unexpectedly.\n");
    free(buf);
    return -1;
  }
  // message received
  buf[rcvd] = '\0';

  if ((tmp = strtok(buf, " \t\r\n")) == NULL) {
    fprintf(stderr, "ERROR: missing HTTP method\n");
    free(buf);
    return -1;
  }
  http_req.method = strdup(tmp);
  if ((tmp = strtok(NULL, " \t")) == NULL) {
    fprintf(stderr, "ERROR: missing HTTP uri\n");
    free(buf);
    return -1;
  }
  http_req.uri = strdup(tmp);
  if (http_req.uri[strlen(http_req.uri) - 1] == '/')
    http_req.uri[strlen(http_req.uri) - 1] = '\0';
  if ((tmp = strtok(NULL, " \t\r\n")) == NULL) {
    fprintf(stderr, "ERROR: missing HTTP protocol\n");
    free(buf);
    return -1;
  }
  http_req.prot = strdup(tmp);
  http_req.payload_size = 0;
  http_req.payload = NULL;

  if (strcmp(http_req.method, "GET") != 0 && strcmp(http_req.method, "POST") != 0) {
    fprintf(stderr, "ERROR: Invalid request: %s \x1b[32m + [%s] %s\x1b[0m\n",
            http_req.prot, http_req.method, http_req.uri);
    free(buf);
    return -1;
  }
  // Now the request is valid and shall be responded
  fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", http_req.method, http_req.uri);

  http_req.qs = strchr(http_req.uri, '?');

  if (http_req.qs) {
    *(http_req.qs)++ = '\0'; // split URI
  } else {
    http_req.qs = http_req.uri - 1; // use an empty string
  }

  struct header_s *h = reqhdr;
  char *t, *t2;
  while (h < reqhdr + 31) {
    char *k, *v, *t;
    
    k = strtok(NULL, "\r\n: \t");
    if (!k)
      break;
    
    v = strtok(NULL, "\r\n");
    while (*v && *v == ' ')
      v++;
    
    h->name = k;
    h->value = v;
    h++;
    fprintf(stderr, "[H] %s: %s\n", k, v);
    t = v + 1 + strlen(v);
    if (t[1] == '\r' && t[2] == '\n')
      break;
  }
  t = strtok(NULL, "\r\n"); // now the *t shall be the beginning of user payload
  t2 = request_header("Content-Length"); // and the related header if there is
  if (t2) {
    http_req.payload = t;
    http_req.payload_size = t2 ? atol(t2) : (rcvd - (t - buf));
  }

  // bind clientfd to stdout, making it easier to write
  clientfd = clients[n];
  //dup2(clientfd, STDOUT_FILENO);

  /*
   * Call router
   * For URI supporting files upload, the router will only register upload
   * callbacks without processing any data
   */
  route(clientfd, &http_req);

  /*
   * If a payload exist, run the callback to process the data, then run another
   * callback when everything is done (typically output HTTP redirect)
   */
  if (http_req.payload_size) { 
    if (http_req.payload_cb) {
      bytes_read = buf_size - (http_req.payload - buf);
      http_req.packet_size = bytes_read;
      http_req.bytes_read = bytes_read;

      do {
        // Callback to process data
        http_req.payload_cb(&http_req);
        if (http_req.complete)
          break;
        rcvd = recv(clients[n], buf, buf_size, 0);

        http_req.payload = buf;
        http_req.packet_size = rcvd;
        http_req.bytes_read += rcvd;
        http_req.count++;
      } while (!http_req.complete);
      fprintf(stderr, "Done\n");
    }

    if (http_req.finish_cb)
      http_req.finish_cb(&http_req);
    if (http_req.name)
      free(http_req.name);
    http_req.name = NULL;
  }
  //write(clientfd, "\nasdf",5 );
  
  close(clientfd);

  // tidy up
  //fflush(stdout);
  //shutdown(STDOUT_FILENO, SHUT_WR);
  //close(STDOUT_FILENO);

  free(buf);
  if (http_req.boundary)
    free(http_req.boundary);
  http_req.boundary = NULL;

  return 0;
}

struct header_s* get_parameter_list(const char* query_string){
	char* query = (char*)malloc(sizeof(char)*(strlen(query_string)+1));
	strcpy(query, query_string);
	query[strlen(query_string)] = '\0';
		
	char** parameter_list;
	if(strcmp(query, "") == 0){
		parameter_list = (char**)malloc(sizeof(char*) * 1);
		parameter_list[0] = NULL;
	}
	else parameter_list = str_split(query, '&');
		
	int n = 0;
	while(parameter_list[n])++n;
	
	struct header_s* query_parameter = (header_s*)malloc(sizeof(struct header_s) * (n+1));
	
	for(int i=0; parameter_list[i]; ++i){
		char* equal_sign_pos = strchr(parameter_list[i], '=');
		query_parameter[i].name = parameter_list[i];
		if(equal_sign_pos){
			*equal_sign_pos = '\0';
			query_parameter[i].value = equal_sign_pos+1;
		}
		else query_parameter[i].value = "";
	}
	
	query_parameter[n].name = NULL;
	free(parameter_list);
	return query_parameter;	
	
}

void destruct_query_parameter(struct header_s* query_parameter){
	int n=0;
	while(query_parameter[n].name){
		free(query_parameter[n].name);
		//free(query_parameter[n].value);
		++n;
	}
	free(query_parameter);
}

char* get_param_value(struct header_s* query_parameter, const char* name){
	int n=0;
	while(query_parameter[n].name){
		if(strcmp(query_parameter[n].name, name) == 0)return query_parameter[n].value;
		++n;	
	}
	return NULL;
}

void respond_http(int fd, const char* status, const char* body){
	//generate http header:
	dprintf(fd, "HTTP/1.1 %s\r\n", status);
	dprintf(fd, "Connection: close\r\n");
	dprintf(fd, "Content-Type: application/json;\r\n");
	dprintf(fd, "\r\n");
	dprintf(fd, "%s", body);
}