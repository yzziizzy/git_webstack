#ifndef __GWS__sys_h__
#define __GWS__sys_h__

#include <sys/stat.h>

#include "cproc.h"



char get_file_type(char* path);
int is_dir(char* path);
int is_file(char* path);
int file_doesnt_exist(char* path);


char* sysstring(char* cmdline);

int file_append_line(char* path, char* line);
int file_write_string(FILE* f, char* s, long n);



#endif // __GWS__sys_h__
