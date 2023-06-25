#ifndef __GWS__scgi_h__
#define __GWS__scgi_h__


#include "net.h"


typedef struct {
	char* key, *value;
	long numval;
} req_header;

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
	
	VEC(req_header) headers;
	
} request;


server_t* scgi_create(int port);



#endif // __GWS__scgi_h__
