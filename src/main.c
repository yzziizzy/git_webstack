#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#include "sti/sti.h"

#include "net.h"
#include "scgi.h"
#include "git_browse.h"
#include "init.h"

#include "strlist.h"


const int g_system_version = 1;

static volatile sig_atomic_t g_shutdown = 0; 

void sigINT(int sig) {
	g_shutdown = 1;
}


int main(int argc, char* argv[]) {
	
	
    struct sigaction act;
    act.sa_handler = sigINT;
    sigaction(SIGINT, &act, NULL);
	
	memtricks_init();

	struct sigaction a;
	memset(&a, 0, sizeof(a));
	a.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &a, NULL);

	if(argc < 2) {
		printf("usage: %s <path_to_repos>\n", argv[0]);
		exit(1);
	}
	
	char do_verify = 0;
	
	char* path_to_init = NULL;
	char* email_to_init = NULL;
	char* username_to_init = NULL;
	char* repos_path = NULL;
	
	char* src_username = NULL;
	char* src_repo = NULL;
	char* dst_username = NULL;
	char* dst_repo = NULL;
	
	char* issue_username = NULL;
	char* issue_file_path = NULL;
	char* target_repo = NULL;
	char* repo_to_create = NULL;
	char* repo_to_rm = NULL;
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
				
				if(0 == strcmp(cmd, "help")) {
					// next arg is the target path
					path_to_init = argv[++ai];
					if(path_to_init == NULL) {
						fprintf(stderr, 
							"Commands:\n"
							"\t--init <path_to_initialize>\n"
							"\t<base_path> --verify \n"
							"\t<base_path> --add-user <username> <email>\n"
							"\t<base_path> --add-repo <username> <repo_name>\n"
							"\t<base_path> --fork <src_username> <src_repo> <dst_username> [<dst_repo>] \n"
							"\t<base_path> --new-issue <posting_user> <repo_username>/<repo> <issue_file>\n"
							"\t<base_path> --add-comment <posting_user> <repo_username>/<repo>/<issue> <comment_file>\n"
						);
						exit(1);
					}
				}
				else if(0 == strcmp(cmd, "init")) {
					// next arg is the target path
					path_to_init = argv[++ai];
					if(path_to_init == NULL) {
						fprintf(stderr, "Usage: %s --init <path_to_initialize>\n", argv[0]);
						exit(1);
					}
				}
				else if(0 == strcmp(cmd, "verify")) {
					// next arg is the target path
					do_verify = 1;
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
				else if(0 == strcmp(cmd, "fork")) {
					// next arg is the target path
					if(argc >= ai + 2) {
						src_username = argv[++ai];
						src_repo = argv[++ai];
						dst_username = argv[++ai];
						if(ai < argc) {
							dst_repo = argv[++ai];
						}
					}
					if(!src_username || !src_repo || !dst_username) {
						fprintf(stderr, "Usage: %s --fork <src_username> <src_repo> <dst_username> [<dst_repo>]\n", argv[0]);
						exit(1);
					}
				}
				else if(0 == strcmp(cmd, "rm-repo")) {
					// next arg is the target path
					if(argc >= ai + 2) {
						target_username = argv[++ai];
						repo_to_rm = argv[++ai];
					}
					if(!target_username || !repo_to_rm) {
						fprintf(stderr, "Usage: %s --fork <src_username> <src_repo> <dst_username> [<dst_repo>]\n", argv[0]);
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
	else if(do_verify) {
		verify_all_users(repos_path);
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
	else if(repo_to_rm) {
		delete_repo(repos_path, target_username, repo_to_rm);
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
	else if(src_username && src_repo && dst_username) {
		if(!dst_repo) dst_repo = src_repo;
		
		fork_repo(repos_path, src_username, src_repo, dst_username, dst_repo);
		
		return 0;
	}
	
	
	

	repo_meta* rm = calloc(1, sizeof(*rm));
	rm->path = strdup(repos_path);//"/home/izzy/projects"; // because the executable is running from the project's repo
	rm->static_asset_path = "./webstatic";
	rm->source_uri_part = "src";
	rm->pulls_uri_part = "pulls";
	rm->wiki_uri_part = "wiki";
	rm->issues_uri_part = "issues";
	
	
	struct uri_pair paths[] = {
		{"", do_site_homepage}, // static assets
		{"_", NULL}, // static assets
		{"%u", do_project_index}, // user profile
		{"u/%u", NULL}, // user meta
		{"u/%u/newrepo", NULL},// create repo
		{"%u/%r", do_project_homepage}, // repo homepage
		{"%u/%r/src", NULL}, // branch list
		{"%u/%r/src/%b", NULL}, // root dir listing of branch/commit
		{"%u/%r/src/%b/%f", do_src_view}, // source view of file
		{"%u/%r/status", NULL}, // repo meta
		{"%u/%r/issues", do_project_issues}, // repo meta
		{"%u/%r/issues/%i", do_project_issues}, // repo meta
		{"%u/%r/settings", NULL}, // repo meta
		{NULL, NULL},
	};
	
	rm->uri_tree = init_uri_tree(paths);
	sort_uri_tree(rm->uri_tree);
	print_uri_tree(rm->uri_tree, 0);

	
	
	int version = get_system_version(repos_path);
	if(version < g_system_version) {
		printf("Filesystem out of date. Updating...\n");
		verify_all_users(repos_path);
		write_system_version(repos_path, g_system_version);
		printf("  Done\n");
	}
	else {
		printf("Verifying repos...\n");
		verify_all_users(repos_path);
	}
	
	scgi_server* srv = scgi_create(4999, rm, git_browse_handler);
	srv->handler = git_browse_handler;
	srv->user_data = rm;
	
	printf("Starting server:\n");
	printf("  port: 4999\n");
	printf("  root: %s\n", rm->path);
	printf("\n");

	
	while(!g_shutdown) {
		
		server_tick(srv->srv, 100);
//		sleep(1);
	}
	
	printf("Shutting down...\n");

	close(srv->srv->epollfd);
	VEC_EACH(&srv->srv->cons, i, con) {
	
	}
	VEC_FREE(&srv->srv->cons);
	free(srv->srv);
	free(srv);
	free(rm->path);
	free(rm);
	
	

	return 0;
}









