#include <stdlib.h>
#include <string.h>

#include "strlist.h"


#define check_alloc(x) \
	if((x)->len + 1 >= (x)->alloc) { \
		(x)->alloc *= 2; \
		(x)->entries = realloc((x)->entries, (x)->alloc * sizeof(*(x)->entries)); \
	}
	

void strlist_init(strlist* sl) {
	sl->len = 0;
	sl->alloc = 32;
	sl->entries = malloc(sl->alloc * sizeof(*sl->entries));
}

strlist* strlist_new() {
	strlist* sl = malloc(sizeof(*sl));
	strlist_init(sl);
	return sl;
}

void strlist_push(strlist* sl, char* e) {
	check_alloc(sl);
	sl->entries[sl->len++] = e;
	sl->entries[sl->len] = 0;
}


void strlist_free(strlist* sl, char freeSelf) {
	for(int i = 0; i < sl->len; i++) {
		free(sl->entries[i]);
	}
	
	if(freeSelf) free(sl);
}




// splits on whitespace
char** str_split(char* in, char* splitters) {
	char* e;
	int alloc = 32;
	int len = 0;
	char** list = malloc(alloc * sizeof(*list)); 
	
	for(char* s = in; *s;) {
		e = strpbrk(s, splitters);
		if(!e) e = s + strlen(s);
		
		if(len >= alloc - 1) {
			alloc *= 2;
			list = realloc(list, alloc* sizeof(*list));
		}
		
		list[len++] = strndup(s, e - s);
		
		e += strspn(e, splitters);
		s = e;
	}
	
	list[len] = NULL;
	
	return list;
}


void free_strpp(char** l) {
	for(char** s = l; *s; s++) free(*s);
	free(l);
}



