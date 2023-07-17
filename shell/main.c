#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cproc.h"

#define p(...) \
do {\
	fprintf(f, __VA_ARGS__); \
	fflush(f); \
} while(0);


FILE* f;

int hexdigit(char c) {
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;
	return 0;
}


int decode_line(char* buf) {
	int len = 0;
	len += hexdigit(buf + 3) * 0x1; 
	len += hexdigit(buf + 2) * 0x10; 
	len += hexdigit(buf + 1) * 0x100; 
	len += hexdigit(buf + 0) * 0x1000; 
	
	
	return 0;
}



int main(int argc, char* argv[]) {
	
	unlink("/tmp/lol.txt");
	f = fopen("/tmp/lol.txt", "wb");
//	FILE* f2 = fopen("/tmp/wut.txt", "wb");
	
	if(!f ) {
		printf("could not open log file\n");
		exit(2);
	}

	for(int i = 0; i < argc; i++) {
		fprintf(f, "%d: %s\n", i, argv[i]);
	}
	fflush(f);
	
	int BUFSZ = 8192;
	char buf[BUFSZ];
//	
//	fprintf(f, "ttyname: %s\n", ttyname(STDOUT_FILENO));
//	fflush(f);
//	
//	int mypty = open(ttyname(STDOUT_FILENO), O_RDWR);
//	if(mypty == -1) {
//		fprintf(f, "\n tty open error: %s\n", strerror(errno));
//		fflush(f);
//		exit(1);
//	}
//	
	fcntl(STDOUT_FILENO, F_SETFL, fcntl(STDOUT_FILENO, F_GETFL) | O_NONBLOCK);
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
//	fcntl(mypty, F_SETFL, fcntl(mypty, F_GETFL) | O_NONBLOCK);

	
	struct child_process_info* cpi = exec_cmdline_pipe(argv[2]);
	
	
	
	int limit = 1000;
	while(1) {
		
//		fprintf(f, "pre stdin read\n");
//		fflush(f);
		
		
		
		int bw;
		int totalWritten = 0;
		do {
			bw = read(cpi->child_stdout, buf, BUFSZ);
			totalWritten += bw;
//			p("read %d\n", bw)
			if(bw > 0) {
				fwrite(buf, 1, bw, f);
			
				fprintf(f, "\n%d bytes read from the child pty\n", bw);
				fflush(f);
				
				fprintf(f, "pre stdout write of %d bytes\n", bw);
				fflush(f);
				
				/*
				for(int j = 0; j < bw; j++) {
					if(EOF == putchar(buf[j])) {
						fprintf(f, "\nputchar write error: %s\n", strerror(errno));
						fflush(f);
						exit(1);
					}
				
				}*/
				
				int bww = 0;
				while(bww < bw) {
					bww += fwrite(buf + bww, 1, bw - bww, stdout);
				}
	//			int bww = write(STDOUT_FILENO, buf, bw);
	//			if(bww < 0) {
	//				fprintf(f, "\nstdout write error: %s\n", strerror(errno));
	//				fflush(f);
	//				break;
	//			}
				
				fprintf(f, "(%d bytes written)\n", bw);
				fflush(f);
	//			fsync(STDOUT_FILENO);
//				fprintf(f, "flushing finished\n");
//				fflush(f);
			}
			else if(bw < 0) {
				if(errno == EIO) {
				//	fprintf(f, "\npty IO Ferror: %s\n", strerror(errno));
			//		fflush(f);
				}
				else if(errno != EAGAIN && errno != EWOULDBLOCK) {
					fprintf(f, "\npty read Verror: %s\n", strerror(errno));
					fflush(f);
					exit(1);
				}
				//p("Zerror: %d - %s\n", errno, strerror(errno))
				break;
			}
			else {
				fprintf(f, "zero bytes \n");
				fflush(f);
			
			}
			
		} while(bw != 0);
		
		if(totalWritten > 0) {
			fprintf(f, "flushing \n");
					fflush(f);
			fflush(stdout);		
			fprintf(f, "flushing finished\n");
					fflush(f);
		}
		
		int br = read(STDIN_FILENO, buf, BUFSZ);
	
		if(br > 0) {
			fwrite(buf, 1, br, f);
			fprintf(f, "\n%d bytes read from shell stdin\n",br);
			fflush(f);
		
			fprintf(f, "pre pty write\n");
			fflush(f);
			
			int brr = 0;
			while(brr < br) {
				brr += write(cpi->child_stdin, buf + brr, br - brr);
			}
			fsync(cpi->child_stdin);
			
			fprintf(f, "postwrite, pre pty read\n");
			fflush(f);
		}
		else if(br < 0) {
			if(errno == EIO) {
			//	p("stdin read Werror: %d - %s\n", errno, strerror(errno))
			}
			else if(errno != EAGAIN) {
				fprintf(f, "\nstdin read Qerror: %s\n", strerror(errno));
				fflush(f);
				break;
			}
			
		}
		
//		fprintf(f, "checking\n");
//		fflush(f);
		int status;
		int pid = waitpid(cpi->pid, &status, WNOHANG);
		if(pid != 0) {
			fprintf(f, "[pid = %d]exiting with status %d\n", pid, WEXITSTATUS(status));
			fflush(f);
			break;
		}
		
//		fprintf(f, "done checking\n");
//		fflush(f);
//		usleep(10);
		
//		exit(1);
//		fprintf(f, ".");
//			fflush(f);
		if(0 && --limit <= 0) {
			fprintf(f, "limit hit\n");
			fflush(f);
			break;
		}
	}
	

	fclose(f);

	kill(cpi->pid, SIGKILL);



	return 0;
}





