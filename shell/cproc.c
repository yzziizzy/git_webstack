#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>


#include "cproc.h"






void free_cpi(struct child_process_info* cpi, char freeOB) {
	if(cpi->output_buffer && freeOB) free(cpi->output_buffer);
	
	if(cpi->f_stdin) fclose(cpi->f_stdin);
	if(cpi->f_stdout) fclose(cpi->f_stdout);
	if(cpi->f_stderr) fclose(cpi->f_stderr);
	
	if(cpi->child_stdin) close(cpi->child_stdin);
	if(cpi->child_stdout) close(cpi->child_stdout);
	if(cpi->child_stderr) close(cpi->child_stderr);
	
	if(cpi->pty) close(cpi->pty);
	
	free(cpi);
}


void read_cpi(struct child_process_info* cpi) {

	while(1) {
		if(cpi->buf_len > cpi->buf_alloc - 128) {
			cpi->buf_alloc *= 2;
			cpi->output_buffer = realloc(cpi->output_buffer, cpi->buf_alloc * sizeof(*cpi->output_buffer));
		}
		
		int ret = read(cpi->pty, cpi->output_buffer + cpi->buf_len, cpi->buf_alloc - cpi->buf_len - 1);
		if(ret > 0) cpi->buf_len += ret;
		else return;
	}
	
	return ;
}





int execute_mt(strlist* cmds, int threads, struct child_process_info*** cpis) {
	int ret = 0;
	int running = 0;
	
	struct child_process_info** procs = calloc(1, cmds->len * sizeof(*procs));
	if(cpis) *cpis = procs;
	
	
	int waiting = cmds->len;
	while(waiting > 0) {
		for(int i = 0; i < cmds->len; i++) {	
			
			// keep the cores full
			if(!procs[i] && running < threads) {
				procs[i] = exec_cmdline_pipe(cmds->entries[i]);
				procs[i]->state = 'r';
				running++;
			}
			
			if(!procs[i] || procs[i]->state == 'd') continue;
			
			
			read_cpi(procs[i]);	
			
			int status;
			// returns 0 if nothing happened, -1 on error, childpid if it exited
			int pid = waitpid(procs[i]->pid, &status, WNOHANG);
			if(pid != 0) {
				procs[i]->state = 'd';
				waiting--;
				running--;
				
				
				read_cpi(procs[i]);
				
				procs[i]->output_buffer[procs[i]->buf_len] = 0;
				procs[i]->exit_status = WEXITSTATUS(status);
				
				if(!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
//					printf("error on pid %d/%d = %d\n", pid, procs[i]->pid, WEXITSTATUS(status));	
//					printf("%s\n", procs[i]->output_buffer);
					ret = 1;
				}
			}
			
			
		}
		
		
		usleep(100);
	}
		
	
	return ret;
}



static int count_args(char* str) {
	int n = 0;
	char inspace = 1;
	char inquote = 0;
	char last_was_slash = 0;
	
	for(char* s = str; *s; s++) {
		if(*s == ' ') {
			if(!last_was_slash && !inquote) {
				if(!inspace) {
					inspace = 1;
				}
			}
			
			last_was_slash = 0;
		}
		else {
			if(inspace) {
				n++;
				inspace = 0;
			}
			
			if(*s == '\\') {
				if(!last_was_slash) last_was_slash = 1;
			}
			else {
				
				if(*s == '"') {
					if(!last_was_slash) {
						inquote = !inquote;
					}
				}
				
				last_was_slash = 0;
			}
		}
	}
	
	return n;
}

static char** split_args(char* cmdline, int* nargs_out) {
	int nargs = count_args(cmdline);
	
	char** args = malloc((nargs + 1) * sizeof(char*));
	
	int n = 0;
	char inspace = 1;
	char inquote = 0;
	char last_was_slash = 0;
	char* start = NULL;
	
	char* s;
	char* end = NULL;
	for(s = cmdline; *s; s++) {
		if(*s == '\'') continue;
		if(*s == ' ') {
			if(!last_was_slash && !inquote) {
				if(!inspace) {
					inspace = 1;
					end = s;
				}
			}
			
			last_was_slash = 0;
		}
		else {
			if(inspace) {
				if(start) {
					args[n++] = strndup(start, end - start);
				}
				start = s;
				inspace = 0;
			}
			
			if(*s == '\\') {
				if(!last_was_slash) last_was_slash = 1;
			}
			else {
				
				if(*s == '"') {
					if(!last_was_slash) {
						inquote = !inquote;
					}
				}
				
				last_was_slash = 0;
			}
			
			end = s;
		}
	}
	
	if(start != s && end != start) {
		args[n++] = strndup(start, end - start + (*end != ' '));
	}
	
	args[nargs] = NULL;
	if(nargs_out) *nargs_out = nargs;
	
	return args;
}


struct child_process_info* exec_cmdline_pipe(char* cmdline) {
	
	
//	char* test[] = {
//		"   a  ",
//		"   a b ",
//		"   aaa bb ",
//		"   aaa bb",
//		"   a b c ",
//		"   a\\ \\ b c ",
//		"\\   a b c ",
//		"\"   a b\" c ",
//		"\"   a\\ b\" c ",
//		"\"   a\\ b\"\\ c ",
//		"\"   a\\ b\"\\ c",
//		NULL,
//	};
//	
//	for(char** x = test; *x; x++) {
//		printf("[%s] = %d\n", *x, count_args(*x));
//		char** a = split_args(*x, NULL);
//		for(int i = 0; a[i]; i++) {
//			printf(" %d = [%s]\n", i, a[i]);
//		}
//	}
//	
	
	
	
	char** args = split_args(cmdline, NULL);

	struct child_process_info* cpi = exec_process_pipe(args[0], args);
//	printf("executing '%s'\n", cmdline);
//	for(char** s = args; *s; s++) printf("%ld - '%s'\n", (s-args), *s);
	free_strpp(args);
	
	return cpi;
}



struct child_process_info* exec_process_pipe(char* exec_path, char* args[]) {
	int in[2]; // io pipes
	int out[2];
	int err[2];
	
	const int RE = 0;
	const int WR = 1;
	
	// 0 = read, 1 = write
	
	if(pipe(in) < 0) {
		return NULL;
	}
	if(pipe(out) < 0) {
		close(in[0]);
		close(in[1]);
		return NULL;
	}
	if(pipe(err) < 0) {
		close(in[0]);
		close(in[1]);
		close(out[0]);
		close(out[1]);
		return NULL;
	}
	
	errno = 0;
	
	int childPID = fork();
	if(childPID == -1) {
		
		fprintf(stderr, "failed to fork trying to execute '%s'\n", exec_path);
		perror(strerror(errno));
		return NULL;
	}
	else if(childPID == 0) { // child process
		
		// redirect standard fd's to the pipe fd's 
		if(dup2(in[RE], fileno(stdin)) == -1) {
			printf("failed 1\n");
			exit(errno);
		}
		if(dup2(out[WR], fileno(stdout)) == -1) {
			printf("failed 2\n");
			exit(errno);
		}
		if(dup2(err[WR], fileno(stderr)) == -1) {
			printf("failed 3\n");
			exit(errno);
		}
		
		// close original fd's used by the parent
		close(in[0]);
		close(in[1]);
		close(out[0]);
		close(out[1]);
		close(err[0]);
		close(err[1]);
		
		
//		fcntl(in[WR], F_SETFL, fcntl(in[WR], F_GETFL) | O_NONBLOCK);
		
		// die when the parent does (linux only)
		prctl(PR_SET_PDEATHSIG, SIGHUP);
		
		// swap for the desired program
		execvp(exec_path, args); // never returns if successful
		
		fprintf(stderr, "failed to execute '%s'\n", exec_path);
		exit(1); // kill the forked process 
	}
	else { // parent process
		
		// close the child-end of the pipes
		struct child_process_info* cpi;
		cpi = calloc(1, sizeof(*cpi));
		
		cpi->child_stdin = in[WR];
		cpi->child_stdout = out[RE];
		cpi->child_stderr = err[RE];
		//cpi->f_stdin = fdopen(cpi->child_stdin, "wb"); // disabled for git clone debugging
		//cpi->f_stdout = fdopen(cpi->child_stdout, "rb");
		//cpi->f_stderr = fdopen(cpi->child_stderr, "rb");
		
		// set to non-blocking
		fcntl(cpi->child_stdout, F_SETFL, fcntl(cpi->child_stdout, F_GETFL) | O_NONBLOCK);
		fcntl(cpi->child_stderr, F_SETFL, fcntl(cpi->child_stderr, F_GETFL) | O_NONBLOCK);
		
		close(in[0]);
		close(out[1]); 
		close(err[1]); 
		
		
		cpi->pid = childPID;
		
// 		int status;
		// returns 0 if nothing happened, -1 on error
// 		pid = waitpid(childPID, &status, WNOHANG);
		
		return cpi;
	}
	
	return NULL; // shouldn't reach here
}
