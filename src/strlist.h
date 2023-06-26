#ifndef __GWS__strlist_h__
#define __GWS__strlist_h__




typedef struct strlist {
	int len;
	int alloc;
	char** entries;
} strlist;



void strlist_free(strlist* sl, char freeSelf);
void strlist_push(strlist* sl, char* e);
strlist* strlist_new();
void strlist_init(strlist* sl);

char* join_str_list(char* list[], char* joiner);

char** str_split(char* in, char* splitters);
void free_strpp(char** l);




#endif // __GWS__strlist_h__
