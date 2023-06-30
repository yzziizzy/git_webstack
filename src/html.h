#ifndef __GWS__html_h__
#define __GWS__html_h__


#include "net.h"
#include "sti/sti.h"


void html_header(connection_t* con);
void html_footer(connection_t* con);

char* html_encode(char* src, ssize_t len);
char* c_ws_escape(char* raw);

#define http302(con, ...) http302_(con, PP_NARG(__VA_ARGS__), __VA_ARGS__)
void http302_(connection_t* con, int nargs, ...);

#define cw(...) cw_(con, PP_NARG(__VA_ARGS__), __VA_ARGS__)
void cw_(connection_t* con, int nargs, ...);

#define cnw(x, l) connection_write(con, x, l)



#endif // __GWS__html_h__
