// SPDX-License-Identifier:  GPL-3.0-or-later

/* TODO: refactor anything here, make it more object oriented*/

#ifndef _HTTPD_H___
#define _HTTPD_H___

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "string_ext.hpp"

//Server control functions

int serve_forever(const char *PORT);

// Client request

char *request_header(const char *name);

struct file_fifo_s *file_push(struct file_fifo_s *fifo, char *name, size_t offset);
struct file_fifo_s *file_pop(struct file_fifo_s *fifo);

struct header_s {
  char *name, *value;
};
struct header_s *request_headers(void);

struct file_fifo_s {
  char *fname;
  char mode[2];
  size_t offset;
  size_t size;
  struct file_fifo_s *next;
};

struct http_req_s {
  char *method;
  char *uri;
  char *qs;
  char *prot;
  char *buf;
  char *boundary;
  char *name;
  char *payload;
  size_t payload_size;
  struct file_fifo_s *files;
  unsigned int complete;
  unsigned int count;
  unsigned int cur;
  unsigned int bytes_read;
  unsigned int packet_size;
  int (*payload_cb)(struct http_req_s *http_req);
  int (*finish_cb)(struct http_req_s *http_req);
};
// user shall implement this function

void route(int file, struct http_req_s *http_req);

// some interesting macro for `route()`
#define ROUTE_START() if (0) {
#define ROUTE(METHOD, URI)                                                     \
  }                                                                            \
  else if (strncmp(URI, http_req->uri, strlen(http_req->uri)) == 0 && strcmp(METHOD, http_req->method) == 0) {
#define ROUTE_URI(URI)                                                         \
  }                                                                            \
  else if (strstr(http_req->uri, URI) == http_req->uri) {
#define ROUTE_BEGIN_WITH(METHOD, URI)                                          \
  }                                                                            \
  else if (strstr(http_req->uri, URI) == http_req->uri && strcmp(METHOD, http_req->method) == 0) {
#define ROUTE_ALL(METHOD, URI)                                                 \
  }                                                                            \
  else if (strstr(http_req->uri, URI) != NULL && strcmp(METHOD, http_req->method) == 0) {
#define ROUTE_GET(URI) ROUTE("GET", URI)
#define ROUTE_GET_BEGIN_WITH(URI) ROUTE_BEGIN_WITH("GET", URI)
#define ROUTE_GET_ALL(URI) ROUTE_ALL("GET", URI) // containing
#define ROUTE_POST(URI) ROUTE("POST", URI)
#define ROUTE_POST_BEGIN_WITH(URI) ROUTE_BEGIN_WITH("POST", URI)
#define AUTH(s) if (is_authorized(http_req->payload, s))
#define ROUTE_END()                                                            \
  }                                                                            \
  else printf("HTTP/1.1 500 Internal Server Error\r\n\r\n"                     \
              "The server has no handler to the request.\r\n");
#define ROUTE_END_URI()                                                        \
  }

#endif

struct header_s* get_parameter_list(const char* query);
char* get_param_value(struct header_s* query_parameter, const char* name);
void destruct_query_parameter(struct header_s* query_parameter);
void respond_http(int fd, const char* status, const char* body);
void* handle_client(void* p_slot);