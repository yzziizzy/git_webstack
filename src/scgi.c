

#include "scgi.h"
#include "net.h"

#include <stdio.h>
#include <ctype.h>




enum {
	REQST_START,
	REQST_GOT_LEN,
	REQST_PARSE_HEADERS,
	REQST_GET_CONTENT,
	REQST_RESPOND,
};


static void on_data(connection_t* con);


static void check_buffer(connection_t* con) {
	scgi_request* req = con->user_data;
	
	if(!req->buf) {
		req->buf = malloc(4096);
		req->buf_alloc = 4096;
		req->buf_remain = 4096;
		return;
	}
	
	req->buf_remain = req->buf_alloc;
	req->buf_alloc *= 2;
	req->buf = realloc(req->buf, req->buf_alloc);
}

static void accepted(server_t* srv, connection_t* con) {
	
	printf("accepted connection\n");
	
	scgi_request* req = calloc(1, sizeof(*req));
	req->srv = srv->user_data;
	
	con->buffer_full = check_buffer;
	con->got_data = on_data;
	con->user_data = req;
	con->buf = &req->buf;
	con->buf_remain = &req->buf_remain;
	
}

static void on_data(connection_t* con) {
	scgi_request* req = con->user_data;
	scgi_server* cgi = con->srv->user_data;
	int len;
	
	while(1) {
		switch(req->state) {
			case REQST_START:
				// look for the netstring encoding of the SCGI header length
				len = req->buf_alloc - req->buf_remain;
				
				char* colon = strnchr(req->buf, ':', len);
				if(!colon) {
					return; // not enough data, somehow...
				}
				
				req->header_netstring_len = strtol(req->buf, NULL, 10);
				req->header_len = req->header_netstring_len - (colon - req->buf) - 1; // minus 1 for the trailing netstring comma
				req->header_offset = (colon - req->buf) + 1;
				
				req->state = REQST_GOT_LEN;
				break;
			
			case REQST_GOT_LEN:
				// wait for all of the headers to arrive
				len = req->buf_alloc - req->buf_remain;
				
				if(len >= req->header_netstring_len) {
					req->state = REQST_PARSE_HEADERS;
					break;
				}
				return;
			
			case REQST_PARSE_HEADERS: {
				
				char* s = req->buf + req->header_offset;
				char* header_end = req->buf + req->header_netstring_len - 1; // minus one for the comma
				req->content_start = header_end + 1;
				
				while(s < header_end) {
					VEC_INC(&req->headers);
					scgi_req_header* header = &VEC_TAIL(&req->headers);
					header->key = strdup(s);
					s += strlen(s) + 1; // skip the null byte
					header->value = strdup(s);
					s += strlen(s) + 1; // skip the null byte
					
					if(isdigit(header->value[0])) {
						header->numval = strtol(header->value, NULL, 10);
					}
					
					if(0 == strcasecmp(header->key, "CONTENT_LENGTH")) {
						req->content_len = header->numval;
						req->total_len = req->content_len + req->header_netstring_len;
					}
					
					//printf("Header: %s = %s\n", header->key, header->value);
				}
				
				req->state = REQST_RESPOND;
				break;
			}
			
			case REQST_GET_CONTENT:
				len = req->buf_alloc - req->buf_remain;
				if(len >= req->total_len) {
					req->state = REQST_RESPOND;
					break;
				}
				return;
			
			case REQST_RESPOND: {
				
				printf("\n start request \n");
				cgi->handler(cgi->user_data, req, con);
				
				connection_close(con);
				free(req->buf);
				VEC_EACHP(&req->headers, i, h) {
					free(h->key);
					free(h->value);
				}
				VEC_FREE(&req->headers);
				free(req);
				
				return;
			}
		}
	}
}



scgi_server* scgi_create(int port, void* user_data, scgi_handler_fn handler) {
	int epollfd = epoll_create(16);
	
	scgi_server* cgi = calloc(1, sizeof(*cgi));
	cgi->user_data = user_data;
	cgi->handler = handler;
	
	server_t* srv = server_init(epollfd, port);
	srv->on_accept = accepted;
	srv->user_data = cgi;
	
	cgi->srv = srv;
	
	return cgi;
}




