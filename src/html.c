#include <string.h>
#include <stdarg.h>


#include "html.h"



void cw_(connection_t* con, int nargs, ...) {
	va_list va;
	va_start(va, nargs);
	
	for(int i = 0; i < nargs; i++) {
		connection_write(con, va_arg(va, char*), -1);
	}
	
	va_end(va);
}



void html_header(connection_t* con) {
	cw("<html>\n<head><link rel=\"stylesheet\" type=\"text/css\" href=\"/_/main.css\" />\n<script type=\"text/javascript\" src=\"/_/main.js\"></script>\n</head>\n<body>\n");
}

void html_footer(connection_t* con) {
	cw("\n</body>\n</html>\n");
}


void html_repo_header(connection_t* con, git_repo* gr) {
	cw("<div class=\"project-title\"><a href=\"/", gr->owner, "\">", gr->owner, "</a>");
	cw("/<a href=\"/", gr->owner, "/", gr->repo_name, "\">", gr->repo_name, "</a></div>");
}


void http302_(connection_t* con, int nargs, ...) {
	cw("Status: 302\r\nLocation: ");
	
	va_list va;
	va_start(va, nargs);

	for(size_t i = 0; i < nargs; i++) {
		cw(va_arg(va, char*));
	}

	va_end(va);
	
	cw("\r\n\r\n");
}



char* html_encode(char* src, ssize_t len) {
	if(len < 0) len = strlen(src);
	
	int replacements = 0;
	for(int i = 0; src[i] && i < len; i++) {
		if(strchr("><&", src[i])) replacements++;
	}
	
	if(replacements == 0) {
		return strndup(src, len);
	}
	
	char* dst = malloc(len + replacements * 5 + 1);
	
	int d = 0;
	for(int i = 0; src[i] && i < len; i++) {
		switch(src[i]) {
			case '>':
				dst[d++] = '&';
				dst[d++] = 'g';
				dst[d++] = 't';
				dst[d++] = ';';
				break;
			case '<':
				dst[d++] = '&';
				dst[d++] = 'l';
				dst[d++] = 't';
				dst[d++] = ';';
				break;
			case '&':
				dst[d++] = '&';
				dst[d++] = 'a';
				dst[d++] = 'm';
				dst[d++] = 'p';
				dst[d++] = ';';
				break;
			default:
				dst[d++] = src[i];
		}
	}
	
	dst[d] = 0;
	
	return dst;
}




char* c_ws_escape(char* raw) {
	
	long ws = 0;
	long len = 0;
	for(char* s = raw; *s; s++) {
		if(*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') ws++;
		len++;
	}
	
	char* out = malloc(len + 1 + ws);
	
	long j = 0;
	for(long i = 0; i < len; i++, j++) {
		if(raw[i] == ' ') {
			out[j++] = '\\';
			out[j] = ' ';
		}
		else if(raw[i] == '\t') {
			out[j++] = '\\';
			out[j] = 't';
		}
		else if(raw[i] == '\r') {
			out[j++] = '\\';
			out[j] = 'r';
		}
		else if(raw[i] == '\n') {
			out[j++] = '\\';
			out[j] = 'n';
		}
		else {
			out[j] = raw[i];
		}
	}
	
	return out;
}





