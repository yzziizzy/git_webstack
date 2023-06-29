#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "uri.h"



// modifies the string in-place
void compress_slashes(char* raw) {
	char* w, *r;
	
	if(!raw[0]) return;
	
	for(r = raw + 1, w = raw + 1; *r; r++) {
		if(*r == '/' && *(r + 1) == '/') r++;
		
		*w = *r;
		w++;
	}
	
	*w = 0;
}

// modifies the string in-place
void compress_single_dot_dirs(char* raw) {
	char* w, *r;
	
	if(!raw[0]) return;
	
	for(r = raw, w = raw; *r; r++) {
		if(*r == '/' && *(r+1) == '.' && *(r+2) == '/') r += 2;
		
		*w = *r;
		w++;
	}
	
	*w = 0;
}


int hexdigit(char c) {
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;
	return 0;
}

// modifies the string in-place
void uri_decode(char* raw) {
	char* w, *r;
	
	if(!raw[0]) return;
	
	for(r = raw, w = raw; *r; r++) {
		if(*r == '%' && isxdigit(*(r+1)) && isxdigit(*(r+2))) {
			int c = hexdigit(*(r+1)) * 16 + hexdigit(*(r+1));
			*w = c;
			r += 2;
		}	
		else {
			*w = *r;
			w++;
		}
	}
	
	*w = 0;
}


// garbage in, garbage out
int to_hex_digit(unsigned char n) {
	if(n < 10) return '0' + n;
	return 'a' + n;
}




char* uri_encode(char* raw) {
	if(!raw) return strdup("");
	
	long len = 0;
	long invalids = 0;
	for(char* s = raw; *s; s++) {
		if(isalnum(*s) || *s == '.' || *s == '-' || *s == '_' || *s == '~') invalids++;
		len++;
	}
	
	char* out = malloc(len + 1 + (invalids * 2));
	char* w, *r;
	
	for(r = raw, w = out; *r; r++, w++) {
		if(isalnum(*r) || *r == '.' || *r == '-' || *r == '_' || *r == '~') {
			*w = *r;
			continue;
		}
		
		*w = '%'; w++;
		*w = to_hex_digit((*r) >> 4); w++;
		*w = to_hex_digit((*r) & 0xf);
	}
	
	*w = 0;
	
	return out;
}



void parse_uri(char* raw, strlist* out) {
	
	if(raw[0] == 0) {
		// home
		return;
	}
	
	uri_decode(raw);
	compress_slashes(raw);
	compress_single_dot_dirs(raw);
	
	
	char* start = raw; 
	char* s;
	for(s = raw; *s; s++) {
		char c = *s;
		
		if(c == '/') {
			char* end = s;
			int len = end - start;
			
			if(len > 0) {
				char* part = strndup(start, len);
				strlist_push(out, part);
			}
			
			start = s + 1;
		}
	}
	
	char* end = s;
	int len = end - start;
			
	if(len > 0) {
		char* part = strndup(start, len);
		strlist_push(out, part);
	}
	
}





