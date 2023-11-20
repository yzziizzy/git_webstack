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
	sl->entries[0] = 0;
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

// remove and return the first entry
char* strlist_shift(strlist* sl) {
	if(sl->len == 0) return NULL;
	
	char* ret = sl->entries[0];
	
	sl->len--;
	memmove(sl->entries, sl->entries + 1, sl->len * sizeof(*sl->entries));
	
	return ret;
}


void strlist_free(strlist* sl, char freeSelf) {
	for(int i = 0; i < sl->len; i++) {
		free(sl->entries[i]);
	}
	
	if(freeSelf) free(sl);
}


// full deep clone
strlist* strlist_clone(strlist* old) {
	strlist* new = malloc(sizeof(*new));
	new->alloc = old->alloc;
	new->len = old->len;
	new->entries = malloc(new->alloc * sizeof(*new->entries));
	
	for(int i = 0; i < new->len; i++) {
		new->entries[i] = strdup(old->entries[i]);
	}
	
	new->entries[new->len] = NULL;
	
	return new;
}


// full deep clone
strlist* strlist_clone_from(strlist* old, int offset) {
	strlist* new = malloc(sizeof(*new));
	new->alloc = old->alloc;
	new->len = old->len - offset;
	new->entries = malloc(new->alloc * sizeof(*new->entries));
	
	for(int i = 0; i < new->len; i++) {
		new->entries[i] = strdup(old->entries[i + offset]);
	}
	
	new->entries[new->len] = NULL;
	
	return new;
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



char* join_str_list(char* list[], char* joiner) {
	size_t list_len = 0;
	size_t total = 0;
	size_t jlen = strlen(joiner);
	
	if(list == NULL) return strdup("");
	
	// calculate total length
	for(int i = 0; list[i]; i++) {
		list_len++;
		total += strlen(list[i]);
	}
	
	if(total == 0) return strdup("");
	
	total += (list_len - 1) * jlen;
	char* out = malloc((total + 1) * sizeof(*out));
	
	char* end = out;
	for(int i = 0; list[i]; i++) {
		char* s = list[i];
		size_t l = strlen(s);
		
		if(i > 0) {
			memcpy(end, joiner, jlen);
			end += jlen;
		}
		
		if(s) {
			memcpy(end, s, l);
			end += l;
		}
		
		total += strlen(list[i]);
	}
	
	*end = 0;
	
	return out;
}



