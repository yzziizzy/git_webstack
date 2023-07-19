#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#include "sti/sti.h"

#include "net.h"
#include "scgi.h"
#include "git_browse.h"

#include "strlist.h"


void initialize_path_for_system(char* path, char clobber);
void initialize_user(char* syspath, char* username, char* email, char clobber);
int initialize_repo(char* syspath, char* username, char* reponame);
int create_issue(git_repo* gr, char* issue_username, char* issue_file_path);
int issue_add_comment(git_issue* gi, char* comment_username, char* comment_file_path);



int main(int argc, char* argv[]) {
	memtricks_init();

	struct sigaction a;
	memset(&a, 0, sizeof(a));
	a.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &a, NULL);

	if(argc < 2) {
		printf("usage: %s <path_to_repos>\n", argv[0]);
		exit(1);
	}
	
	char* path_to_init = NULL;
	char* email_to_init = NULL;
	char* username_to_init = NULL;
	char* repos_path = NULL;
	
	char* issue_username = NULL;
	char* issue_file_path = NULL;
	char* target_repo = NULL;
	char* repo_to_create = NULL;
	char* target_username = NULL;
	
	char* comment_username = NULL;
	char* target_issue = NULL;
	char* comment_file_path = NULL;
	
	
	for(int ai = 1; ai < argc; ai++) {
		char c = argv[ai][0];
		
		if(c == '-') {
		
			// long args
			if(argv[ai][1] == '-') {
				char* cmd = argv[ai] + 2;
				
				if(0 == strcmp(cmd, "init")) {
					// next arg is the target path
					path_to_init = argv[++ai];
					if(path_to_init == NULL) {
						fprintf(stderr, "Usage: %s --init <path_to_initialize>\n", argv[0]);
						exit(1);
					}
				}
				else if(0 == strcmp(cmd, "add-user")) {
					// next arg is the target path
					if(argc >= ai + 2) {
						username_to_init = argv[++ai];
						email_to_init = argv[++ai];
					}
					if(!username_to_init || !email_to_init) {
						fprintf(stderr, "Usage: %s --add-user <username> <email>\n", argv[0]);
						exit(1);
					}
				}
				else if(0 == strcmp(cmd, "add-repo")) {
					// next arg is the target path
					if(argc >= ai + 2) {
						target_username = argv[++ai];
						repo_to_create = argv[++ai];
					}
					if(!repo_to_create || !target_username) {
						fprintf(stderr, "Usage: %s --add-repo <username> <repo_name>\n", argv[0]);
						exit(1);
					}
				}
				else if(0 == strcmp(cmd, "new-issue")) {
					// next arg is the target path
					if(argc >= ai + 3) {
						issue_username = argv[++ai];
						target_repo = argv[++ai];
						issue_file_path = argv[++ai];
					}
					if(!issue_file_path || !issue_username || !target_repo) {
						fprintf(stderr, "Usage: %s --new-issue <posting_user> <repo_username>/<repo> <issue_file> \n", argv[0]);
						exit(1);
					}
				}
				else if(0 == strcmp(cmd, "add-comment")) {
					// next arg is the target path
					if(argc >= ai + 3) {
						comment_username = argv[++ai];
						target_issue = argv[++ai];
						comment_file_path = argv[++ai];
					}
					if(!comment_username || !target_issue || !comment_file_path) {
						fprintf(stderr, "Usage: %s --add-comment <posting_user> <repo_username>/<repo>/<issue> <comment_file> \n", argv[0]);
						exit(1);
					}
				}
				else if(0 == strcmp(cmd, "repos")) {
					// next arg is the target path
					repos_path = argv[++ai];
					if(path_to_init == NULL) {
						fprintf(stderr, "Usage: %s --repos <directory_containing_git_repositories>\n", argv[0]);
						exit(1);
					}
				}
				else {
					fprintf(stderr, "Unknown argument: --%s\n", cmd);
					exit(1);
				}
			
			}
			else { // short args
				for(int i = 1; argv[ai][i]; i++) {
					switch(argv[ai][i]) {
					
					
						default:
							fprintf(stderr, "Unknown argument: -%c\n", argv[ai][i]);
							exit(1);
					}
				}
			}
		}
		else {
			repos_path = argv[ai];
		}
	
	}
	
	if(path_to_init) {
		initialize_path_for_system(path_to_init, 0);
		return 0;
	}
	else if(username_to_init) {
		initialize_user(repos_path, username_to_init, email_to_init, 0);
		return 0;
	}
	else if(repo_to_create) {
		initialize_repo(repos_path, target_username, repo_to_create);
		return 0;
	}
	else if(issue_username) {
		git_repo gr = {0};
		if(git_repo_init_short(&gr, repos_path, target_repo)) {
			fprintf(stderr, "No such repo '%s'\n", target_repo);
			return 1;
		}
	
		create_issue(&gr, issue_username, issue_file_path);
		
		free_git_repo(&gr);
		return 0;
	}
	else if(comment_username) {
		git_issue gi = {0};
		if(git_issue_init_short(&gi, repos_path, target_issue)) {
			fprintf(stderr, "No such issue '%s'\n", target_issue);
			return 1;
		}
	
		issue_add_comment(&gi, comment_username, comment_file_path);
		
		free_git_issue(&gi);
		return 0;
	}
	
	
	

	repo_meta* rm = calloc(1, sizeof(*rm));
	rm->path = strdup(argv[1]);//"/home/izzy/projects"; // because the executable is running from the project's repo
	rm->static_asset_path = "./webstatic";
	rm->source_uri_part = "src";
	rm->pulls_uri_part = "pulls";
	rm->wiki_uri_part = "wiki";
	rm->issues_uri_part = "issues";
	
	scgi_server* srv = scgi_create(4999, rm, git_browse_handler);
	srv->handler = git_browse_handler;
	srv->user_data = rm;
	
	printf("Starting server:\n");
	printf("  port: 4999\n");
	printf("  root: %s\n", rm->path);
	printf("\n");

	
	while(1) {
		
		server_tick(srv->srv, 100);
		
//		sleep(1);
	}


	close(srv->srv->epollfd);

	return 0;
}









