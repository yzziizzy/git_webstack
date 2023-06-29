#ifndef __GWS__cproc_h__
#define __GWS__cproc_h__

#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <pty.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "strlist.h"


struct child_process_info {
	int pid;
	char state; // 'q'ueued, 'r'unning, 'd'ead
	int exit_status;
	int pty;
	int child_stdin;
	int child_stdout;
	int child_stderr;
	FILE* f_stdin;
	FILE* f_stdout;
	FILE* f_stderr;
	
	char* output_buffer;
	size_t buf_alloc;
	size_t buf_len;
};


struct job {
	int type; // c = child process, f = function
	volatile int state;
	
	union {
		int (*fn)(void*);
		char* cmd;
	};
	union {
		struct child_process_info* cpi;
		void* user_data;
	};
};


struct child_process_info* exec_cmdline_pipe(char* cmdline);
struct child_process_info* exec_process_pipe(char* exec_path, char* args[]);
int execute_mt(strlist* cmds, int threads, struct child_process_info*** cpis);

void free_cpi(struct child_process_info* cpi, char freeOB);
void read_cpi(struct child_process_info* cpi);

#endif // __GWS__cproc_h__
