
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>

#include "sys.h"
#include "sti/sti.h"


char get_file_type(char* path) {
	struct stat st;
	int res = stat(path, &st);
	if(res) return 0;
	
	if(st.st_mode & S_IFDIR) return 'd';
	if(st.st_mode & S_IFREG) return 'f';
	if(st.st_mode & S_IFLNK) return 'l';
	
	return 'o';
}

int is_dir(char* path) {
	return 'd' == get_file_type(path);
}

int is_file(char* path) {
	return 'f' == get_file_type(path);
}

int file_doesnt_exist(char* path) {
	return 0 == get_file_type(path);
}


char* systemf(char* fmt, ...) {
	va_list va;
	
	va_start(va, fmt);
	size_t n = vsnprintf(NULL, 0, fmt, va);
	char* buf = malloc(n + 1);
	va_end(va);
	
	va_start(va, fmt);
	vsnprintf(buf, n + 1, fmt, va);
	va_end(va);

	printf("system: %s\n", buf);
	char* str = sysstring(buf);
	
	free(buf);
	
	return str;
}


char* sysstring(char* cmdline) {
	struct child_process_info* cpi = exec_cmdline_pipe(cmdline);

	
	while(1) {	
		read_cpi(cpi);	
		
		int status;
		// returns 0 if nothing happened, -1 on error, childpid if it exited
		int pid = waitpid(cpi->pid, &status, WNOHANG);
		if(pid != 0) {
			
			read_cpi(cpi);
			
			cpi->output_buffer[cpi->buf_len] = 0;
			cpi->exit_status = WEXITSTATUS(status);
			
			break;
		}
		
		usleep(100);
	}
	
	char* out = cpi->output_buffer;
	
	if(cpi->exit_status != 0) {
		free(out);
		return NULL;
	}
	
	free_cpi(cpi, 0);
	
	return out;
}


int file_write_string(FILE* f, char* s, long n) {
	if(n == 0) return 0;
	if(n < 0) n = strlen(s);
	
	long bwritten = 0;
	do {
		bwritten = fwrite(s + bwritten, 1, n, f);
		if(bwritten == 0) {
			if(errno != 0) return 1;
		}
		
		n -= bwritten;
	} while(n);
	
	return 0;
}



int file_append_line(char* path, char* line) {
	FILE* f = fopen(path, "wb");
	if(!f) {
		fprintf(stderr, "Error: could not open '%s' for writing.\n", path);
		return 1;
	}
	
	fseek(f, -1, SEEK_END);
	
	char c;
	long fpos = ftell(f);
	if(fpos > 0) {
		c = 0;
		
		fread(&c, 1, 1, f);
		if(c != '\n') {
			fseek(f, 1, SEEK_CUR);
			c = '\n';
			if(1 != fwrite(&c, 1, 1, f)) {
				return 2;
			}
		}
	}
	
	
	file_write_string(f, line, -1);
	
	c = '\n';
	if(1 != fwrite(&c, 1, 1, f)) {
		return 3;
	}
	
	fclose(f);
	
	return 0;
}



// should be a clean path; no extra slashes
char* get_file(char* path) {
	
	// TODO: caching
	
	return read_whole_file(path, NULL);
}




