#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#include "sti/sti.h"

#include "net.h"
#include "scgi.h"
#include "git_browse.h"

#include "strlist.h"




char* tests[] = {
	"",
	"..",
	"/",
	"/../..",
	"/./",
	"///",
	"/sti",
	"///sti",
	"/sti/",
	"/sti//",
	"/sti/sti.h",
	"/s%20ti/.sti.h",
	"/sti/..sti.h",
	"/sti/../sti.h",
	"/sti/two/sti.h",
	"/sti/two//sti.h",
	"/%73ti///two//sti.h",
	"/sti/%2f/two//sti.h",
	"/sti/%2e/two//sti.h",
	"/sti//./two////sti.h",
	NULL,
};


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



void parse_uri(char* raw, strlist* out) {
	
	if(raw[0] == 0) {
		// home
		return;
	}
	
	uri_decode(raw);
	compress_slashes(raw);
	compress_single_dot_dirs(raw);
	
	
	char has_slash = 0;
	char has_period = 0;
	
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



int main(int argc, char* argv[]) {
	struct sigaction a;
	memset(&a, 0, sizeof(a));
	a.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &a, NULL);

	for(char** t = tests; *t; t++) {
		printf("\nbefore: '%s', ", *t);
		char* s = strdup(*t);
		strlist* list = strlist_new();
		parse_uri(s, list);
		printf("after: '%s'\n", s);
		for(int i = 0; i < list->len; i++) {
			printf("  '%s'\n", list->entries[i]);
		}
		
	} 
	return 0;

	if(argc < 2) {
		printf("usage: %s <path_to_repos>\n", argv[0]);
		exit(1);
	}

	repo_meta* rm = calloc(1, sizeof(*rm));
	rm->path = argv[1];//"/home/izzy/projects"; // because the executable is running from the project's repo
	rm->static_asset_path = "./webstatic";
	
	scgi_server* srv = scgi_create(4999, rm, git_browse_handler);
	srv->handler = git_browse_handler;
	srv->user_data = rm;

	
	while(1) {
		
		server_tick(srv->srv, 100);
		
//		sleep(1);
	}


	close(srv->srv->epollfd);

	return 0;
}









