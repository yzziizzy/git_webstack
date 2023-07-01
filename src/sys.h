#ifndef __GWS__sys_h__
#define __GWS__sys_h__

#include <sys/stat.h>

#include "cproc.h"



char get_file_type(char* path);
int is_dir(char* path);
int is_file(char* path);
int file_doesnt_exist(char* path);


char* systemf(char* fmt, ...);
char* sysstring(char* cmdline);

int file_append_line(char* path, char* line);
int file_write_string(FILE* f, char* s, long n);

strlist* list_directories(char* path);

// should be a clean path; no extra slashes
char* get_file(char* path);

char* extract_line(char* src, char* key);

#endif // __GWS__sys_h__
