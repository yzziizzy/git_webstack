
#include <stdio.h>
#include <errno.h>

#include "sti/sti.h"
#include "sys.h"



/* Directory Layout:

[root] /
	+ users
		+ [user]
			+ repos
				+ [repo]
					+ src
					+ meta
					+ issues
					+ pulls
			+ status
				+ wiki
				- following
				- followed_by
				- starred
				- watching
				- contact
				
			+ settings
				- following
				- starred
				- watching
				- emails
				- keys
	+ sysmeta
		- passwd
		- user_emails



*/


// does not handle escaped slashes
int mkdirp(char* path, mode_t mode);

void make_check_dir(char* p, mode_t mode) {
	
	char type = get_file_type(p);
	if(type && type != 'd') {
		fprintf(stderr, "Init error: '%s' already exists and is not a directory.\n", p);
		exit(1);
	}
	
	mkdirp(p, mode);
	
	type = get_file_type(p);
	if(type != 'd') {
		fprintf(stderr, "Init error: '%s' could not be created.\n", p);
		exit(1);
	}
}



void initialize_path_for_system(char* path, char clobber) {


	make_check_dir(path, 0777);

	char* path_users = path_join(path, "users");
	make_check_dir(path_users, 0777);
	
	char* path_sysmeta = path_join(path, "sysmeta");
	make_check_dir(path_sysmeta, 0777);
	
	char* meta_passwd = path_join(path_sysmeta, "passwd");
	fclose(fopen(meta_passwd, "a"));
	
	char* meta_user_emails = path_join(path_sysmeta, "user_emails");
	fclose(fopen(meta_user_emails, "a"));
	
	free(meta_passwd);
	free(meta_user_emails);
	free(path_users);
	free(path_sysmeta);
}



void initialize_user(char* syspath, char* username, char* email, char clobber) {
	
	char type = get_file_type(syspath);
	if(type != 'd') {
		fprintf(stderr, "Init user error: '%s' is not a gitviewer system root directory.\n", syspath);
		exit(1);
	}
	
	char* user_dir = path_join(syspath, "users", username);
	type = get_file_type(user_dir);
	if(type != 0) {
		if(clobber) {
			fprintf(stderr, "Init user error: '%s' already exists. (clobber NYI for safety reasons.)\n", user_dir);
			exit(1);
		}
		else {
			fprintf(stderr, "Init user error: '%s' already exists.\n", user_dir);
			exit(1);
		}
	}
	
	make_check_dir(user_dir, 0777);
	

	char* user_repos = path_join(user_dir, "repos");
	make_check_dir(user_repos, 0777);
	
	char* user_meta = path_join(user_dir, "meta");
	make_check_dir(user_meta, 0777);
	// TODO: git init
	
	
	char* meta_pulls = path_join(user_meta, "pulls");
	make_check_dir(meta_pulls, 0777);
	
	char* meta_issues = path_join(user_meta, "issues");
	make_check_dir(meta_issues, 0777);
	
	char* meta_wiki = path_join(user_meta, "wiki");
	make_check_dir(meta_wiki, 0777);
	
	char* meta_following = path_join(user_meta, "following");
	fclose(fopen(meta_following, "a"));
	
	char* meta_followers = path_join(user_meta, "followers");
	fclose(fopen(meta_followers, "a"));
	
	char* meta_starred = path_join(user_meta, "starred");
	fclose(fopen(meta_starred, "a"));
	
	char* meta_watching = path_join(user_meta, "watching");
	fclose(fopen(meta_watching, "a"));
	
	// TODO: git add/commit
	
//	file_append_line()
	
	free(user_dir);
	free(user_meta);
	free(user_repos);
	free(meta_pulls);
	free(meta_issues);
	free(meta_wiki);
	free(meta_following);
	free(meta_followers);
	free(meta_starred);
	free(meta_watching);
}



int initialize_repo(char* syspath, char* username, char* reponame) {
	char type = get_file_type(syspath);
	if(type != 'd') {
		fprintf(stderr, "Init repo error: '%s' is not a gitviewer system root directory.\n", syspath);
		exit(1);
	}
	
	
	char* user_repos = path_join(syspath, "users", username, "repos");
		if(!is_dir(user_repos)) {
		fprintf(stderr, "Init repo error: '%s' is not a valid username.\n", username);
		exit(1);
	}
	
	char* repo_dir = path_join(user_repos, reponame);
	make_check_dir(repo_dir, 0777);
	
	char* src_dir = path_join(repo_dir, "src.git");
	make_check_dir(src_dir, 0777);
	
	char* pulls_dir = path_join(repo_dir, "pulls");
	make_check_dir(pulls_dir, 0777);
	
	char* issues_dir = path_join(repo_dir, "issues");
	make_check_dir(issues_dir, 0777);
	
	char* issues_op_dir = path_join(issues_dir, "open");
	make_check_dir(issues_op_dir, 0777);
	
	char* issues_cl_dir = path_join(issues_dir, "closed");
	make_check_dir(issues_cl_dir, 0777);
	
	systemf("git --git-dir=%s init --bare --shared=all", src_dir);
	
	return 0;
}

// does not handle escaped slashes
int mkdirp(char* path, mode_t mode) {
	
	char* clean_path = strdup(path);
	
	// inch along the path creating each directory in line
	for(char* p = clean_path; *p; p++) {
		if(*p == '/') {
			*p = 0;
			
			if(p != clean_path) {
				if(mkdir(clean_path, mode)) {
					if(errno != EEXIST) {
						printf("foo '%s'\n", clean_path);				
						goto FAIL;
					}
				}
			}
			
			*p = '/';
		}
	}
	
	// mop up the last dir
	if(mkdir(clean_path, mode)) {
		if(errno != EEXIST) {
			printf("bar\n");
			goto FAIL;
		}
	}
	
	free(clean_path);
	return 0;
	
FAIL:
	free(clean_path);
	return -1;
}

