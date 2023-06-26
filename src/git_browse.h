#ifndef __GWS__git_browse_h__
#define __GWS__git_browse_h__


#include "scgi.h"

typedef struct {
	char* path;

} repo_meta;




void git_browse_handler(void* user_data, scgi_request* req, connection_t* con);




#endif // __GWS__git_browse_h__
