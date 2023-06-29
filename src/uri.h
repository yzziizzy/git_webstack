#ifndef __GWS__uri_h__
#define __GWS__uri_h__

#include "strlist.h"



// modifies the string in-place
void compress_slashes(char* raw);

// modifies the string in-place
void compress_single_dot_dirs(char* raw);

int hexdigit(char c);

// modifies the string in-place
void uri_decode(char* raw);

// returns new memory
char* uri_encode(char* raw);

void parse_uri(char* raw, strlist* out);

#endif // __GWS__uri_h__
