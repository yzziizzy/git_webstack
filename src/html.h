#ifndef __GWS__html_h__
#define __GWS__html_h__


#include "net.h"


void html_header(connection_t* con);
void html_footer(connection_t* con);

char* html_encode(char* src, ssize_t len);
char* c_ws_escape(char* raw);


#define cw(x) connection_write(con, x, -1)
#define cnw(x, l) connection_write(con, x, l)



#endif // __GWS__html_h__
