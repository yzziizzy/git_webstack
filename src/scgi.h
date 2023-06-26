#ifndef __GWS__scgi_h__
#define __GWS__scgi_h__


#include "net.h"


typedef struct {
	char* key, *value;
	long numval;
} scgi_req_header;


struct scgi_server;

typedef struct {
	char* buf;
	size_t buf_alloc;
	size_t buf_remain;
	
	int state;
	
	int header_len;
	int header_netstring_len;
	int header_offset; // first char after the colon
	char* content_start;
	
	long content_len;
	long total_len;
	
	VEC(scgi_req_header) headers;
	
	struct scgi_server* srv;
} scgi_request;

typedef void (*scgi_handler_fn)(void* user_data, scgi_request* req, connection_t* con);
	

typedef struct {
	server_t* srv;
	
	VEC(scgi_request*) requests;
	
	scgi_handler_fn handler;
	
	void* user_data;
} scgi_server;


scgi_server* scgi_create(int port, void* user_data, scgi_handler_fn handler);



#endif // __GWS__scgi_h__
