#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "cproc.h"

#include "../src/sti/sti.h"
#include "../src/git.h"
#include "../src/sys.h"

#define p(...) \
do {\
	fprintf(f, __VA_ARGS__); \
	fflush(f); \
} while(0);

enum {
	NONE, PUSH, PULL,
};


FILE* f;

int hexdigit(char c) {
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;
	return 0;
}


int read_line_len(char* buf) {
	int len = 0;
	len += hexdigit(buf + 3) * 0x1; 
	len += hexdigit(buf + 2) * 0x10; 
	len += hexdigit(buf + 1) * 0x100; 
	len += hexdigit(buf + 0) * 0x1000; 
	
	return len;
}

/*
char get_file_type(char* path) {
	struct stat st;
	int res = stat(path, &st);
	if(res) return 0;
	
	if(st.st_mode & S_IFDIR) return 'd';
	if(st.st_mode & S_IFREG) return 'f';
	if(st.st_mode & S_IFLNK) return 'l';
	
	return 'o';
}//*/
/*
int is_dir(char* path) {
	return 'd' == get_file_type(path);
}
/*
int is_file(char* path) {
	return 'f' == get_file_type(path);
}

int file_doesnt_exist(char* path) {
	return 0 == get_file_type(path);
}
*/


int main(int argc, char* argv[]) {
	
	char* git_path = NULL;
	
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
	
	int mode = NONE;
	
	if(!strncmp(argv[2], "git-receive-pack", strlen("git-receive-pack"))) mode = PUSH;
	else if(!strncmp(argv[2], "git-upload-pack", strlen("git-upload-pack"))) mode = PULL;
	
	if(mode == NONE) {
		p("unknown mode: %s\n", argv[2]);
		exit(1);
	}
	
	
	
	char* raw = strchr(argv[2], ' ');
	raw += strspn(raw, " ");	
	if(!raw) {
		p("missing repo path\n");
		exit(1);
	}
	if(raw[0] == '\'') {
		raw++;
		
		int len = strlen(raw);
		if(raw[len - 1] == '\'') raw[len - 1] = 0;
	}
	
	if(raw[0] == '~') raw++;
	
	
	/*
	URL Format:
	
	git@server:user/meta
	git@server:user/repo.git
	git@server:user/repo/[meta|issues|pulls]
	
	*/
	
	
	
	char* syspath = getenv("GITVIEWER_BASE_DIR");
	if(!syspath) {
		p("no GitViewer base path in ENV\n");
		exit(1);
	}
	
	
	char** parts = strsplit(raw, '/', NULL);
		
	// grab and validate user
	int goodchars = strspn(parts[0], "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
	if(strlen(parts[0]) != goodchars) {
		p("invalid username: %s\n", parts[0]);
		exit(1);
	}
	
	char* username = parts[0];
	
	
	char* user_dir = path_join(syspath, "users", username);
	if(!is_dir(user_dir)) {
		p("user %s does not exist\n", username);
		exit(1);
	}
	
	
	// grab and verify the repo name
	goodchars = strspn(parts[1], "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.");
	if(strlen(parts[1]) != goodchars) {
		p("invalid repo name: %s\n", raw);
		exit(1);
	}
	
	char* reponame = parts[1];
	
	
	
	

	

		
	

	if(parts[2] == NULL) {
		char* ext = strrchr(reponame, '.');
		if(ext && !strcmp(ext, ".git")) {
			
			ext[0] = 0;
			
			git_path = path_join(user_dir, "repos", reponame, "src.git");
			if(!is_dir(git_path)) {
				p("'%s' is not a valid git repo\n", git_path);
				exit(1);
			}
			
			
			
						
			
			// verify permissions with SSH-provided username
			char* ssh_username = getenv("GITVIEWER_SSH_USER");
			if(!ssh_username) {
				p("no username key provided by ssh\n");
				exit(1);
			}
			
			if(mode == PUSH) {
				if(0 != strcmp(ssh_username, username)) {
				
					git_repo gr = {
						.abs_src_path = path_join(user_dir, "repos", reponame, "settings"),
					};
					
					char* pushers_src = git_get_file(&gr, "master", "pushers");
					char** pushers = strsplit_inplace("\n", pushers_src, NULL);
					
					int allowed = 0;
					for(int i = 0; pushers[i]; i++) {
						if(!strcasecmp(ssh_username, pushers[i])) {
							allowed = 1;
							break;
						}
					}	
					
					if(!allowed) {
						p("provided username and ssh username do not match: %s != %s\n", username, ssh_username);
						exit(1);
					}
				}
			
			}
			
			
			
			
			
			
		}
		else {
		
		}
	
	}
	else {
		
		
		
		char* repo_dir = path_join(user_dir, "repos", reponame);
		if(!is_dir(repo_dir)) {
			p("user/repo %s/%s does not exist\n", username, reponame);
			exit(1);
		}
		
		
		// figure out what they are trying to do
		
		
		if(!strcmp(raw, "src.git")) { // regular git access
			git_path = path_join(repo_dir, "src.git");
			if(!is_dir(git_path)) {
				p("'%s' is not a valid git repo\n", git_path);
				exit(1);
			}
			
			
		}
		else {
			p("operation '%s' not supported\n", raw);
			exit(1);
		}
	}
	
	
	if(!git_path) {
		p("no git repo found\n");
		exit(1);
	}
	
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

	
	char* args[3];
	args[2] = NULL;
	args[1] = git_path;
	args[0] = mode == PULL ? "git-upload-pack" : "git-receive-pack";
	
	p("running %s %s\n", args[0], args[1]);
	struct child_process_info* cpi = exec_process_pipe(args[0], args);
	
	char* line = malloc(1024);
	int lineLen = 0;
	int packline_len = 0;
	
	int limit = 1000;
	while(1) {
		
//		fprintf(f, "pre stdin read\n");
//		fflush(f);
		
		
		
		int bw;
		int totalWritten = 0;
		do {
			p("waiting to read\n");
			bw = read(cpi->child_stdout, buf, BUFSZ);
			
			

			
			p("read %d\n", bw)
			if(bw > 0) {
				totalWritten += bw;
			
				memcpy(line + lineLen, buf, bw);
				lineLen += bw;
				line[lineLen] = 0;
				
				if(packline_len == 0) {
					if(lineLen >= 4) {
						packline_len = read_line_len(line);
						p("packline len: %d: \n", packline_len);
					}
				}
				if(packline_len && lineLen >= packline_len) {
					p("packline (len: %d): \n", packline_len);
					
					memmove(line, line + packline_len, lineLen - packline_len);
					lineLen -= packline_len;
					line[lineLen] = 0;
					packline_len = 0;
				}
			
			
				fwrite(buf, 1, bw, f);
			
				p("\n%d bytes read from the child pty\n", bw);
				p("pre stdout write of %d bytes\n", bw);
				
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
				
				p("(%d bytes written)\n", bw);
	//			fsync(STDOUT_FILENO);
//				fprintf(f, "flushing finished\n");
//				fflush(f);
			}
			else if(bw < 0) {
				p("less than one\n")
				if(errno == EIO) {
					p("\npty IO Ferror: %s\n", strerror(errno));
			//		fflush(f);
				}
				else if(errno != EAGAIN && errno != EWOULDBLOCK) {
					p("\npty read Verror: %s\n", strerror(errno));
					exit(1);
				}
				else if(errno == EAGAIN) {
					p("EAGAIN");
					break;
				}
				p("Zerror: %d - %s\n", errno, strerror(errno))
				break;
			}
			else {
				p("zero bytes \n");
			
			}
			
		} while(bw != 0);
		
		if(totalWritten > 0) {
			p("flushing \n");
			fflush(stdout);		
			p("flushing finished\n");
		}
		
		int br = read(STDIN_FILENO, buf, BUFSZ);
	
		if(br > 0) {
			fwrite(buf, 1, br, f);
			p("\n%d bytes read from shell stdin\n",br);
		
			p("pre pty write\n");
			
			int brr = 0;
			while(brr < br) {
				brr += write(cpi->child_stdin, buf + brr, br - brr);
			}
			fsync(cpi->child_stdin);
			
			p("postwrite, pre pty read\n");
		}
		else if(br < 0) {
			if(errno == EIO) {
			//	p("stdin read Werror: %d - %s\n", errno, strerror(errno))
			}
			else if(errno != EAGAIN) {
				p("\nstdin read Qerror: %s\n", strerror(errno));
				break;
			}
			
		}
		
//		fprintf(f, "checking\n");
//		fflush(f);
		int status;
		int pid = waitpid(cpi->pid, &status, WNOHANG);
		if(pid != 0) {
			p("[pid = %d]exiting with status %d\n", pid, WEXITSTATUS(status));
			break;
		}
		
//		fprintf(f, "done checking\n");
//		fflush(f);
//		usleep(10);
		
//		exit(1);
//		fprintf(f, ".");
//			fflush(f);
		if(0 && --limit <= 0) {
			p("limit hit\n");
			break;
		}
	}
	
	p("done\n");
	

	fclose(f);

	kill(cpi->pid, SIGKILL);



	return 0;
}





