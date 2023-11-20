
#include <stdio.h>
#include <errno.h>

#include "sti/sti.h"
#include "sys.h"
#include "git.h"
#include "init.h"



/* Directory Layout:

[root] /
	+ users
		+ [user]
			+ repos
				+ [repo]
					+ src.git
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


int get_system_version(char* syspath) {
	char* path = path_join(syspath, "fs_layout_version");
	char* txt = read_whole_file(path, NULL);
	if(!txt) return -1;
	
	int version = strtol(txt, NULL, 10);
	
	free(txt);
	free(path);
	
	return version;
}


void write_system_version(char* syspath, int version) {
	char* path = path_join(syspath, "fs_layout_version");
	
	FILE* f = fopen(path, "wb+");
	fprintf(f, "%d", version);
	fclose(f);
	
	free(path);
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




typedef struct {
	char* syspath;
	int ret;
} users_verify_info;

static int verify_all_users_cb(char* fullPath, char* fileName, void* data) {
	users_verify_info* info = (users_verify_info*)data;
	info->ret |= verify_user(info->syspath, fileName);
	return 0;
}

int verify_all_users(char* syspath) {
	users_verify_info info = {
		.syspath = syspath,
		.ret = 0,
	};
	
	char* users_dir = path_join(syspath, "users"); 
	recurse_dirs(users_dir, verify_all_users_cb, &info, 0, FSU_EXCLUDE_FILES | FSU_INCLUDE_DIRS | FSU_NO_FOLLOW_SYMLINKS | FSU_EXCLUDE_HIDDEN);
	free(users_dir);
	
	return info.ret;
}



int verify_user(char* syspath, char* username) {
	return verify_user_repos(syspath, username);
}


int verify_repo_structure(char* syspath, char* username, char* reponame) {


	char* repo_dir = path_join(syspath, "users", username, "repos", reponame);
	make_check_dir(repo_dir, 0777);
	
	char* src_dir = path_join(repo_dir, "src.git");
	make_check_dir(src_dir, 0777);
	
	char* worktrees_dir = path_join(repo_dir, "worktrees");
	make_check_dir(worktrees_dir, 0777);
	
	char* pulls_dir = path_join(repo_dir, "pulls");
	make_check_dir(pulls_dir, 0777);
	
	char* issues_dir = path_join(repo_dir, "issues");
	make_check_dir(issues_dir, 0777);
	
//	char* issues_op_dir = path_join(issues_dir, "open");
//	make_check_dir(issues_op_dir, 0777);
//	
//	char* issues_cl_dir = path_join(issues_dir, "closed");
//	make_check_dir(issues_cl_dir, 0777);
	
	char* settings_dir = path_join(repo_dir, "settings");
	if(!git_is_bare_repo(settings_dir)) {
		git_init_bare(settings_dir);
	}
	
	free(repo_dir);
	free(src_dir);
	free(worktrees_dir);
	free(pulls_dir);
	free(issues_dir);
//	free(issues_cl_dir);
//	free(issues_op_dir);
	free(settings_dir);
	
	return 0;
}


typedef struct {
	char* syspath;
	char* username;
	int ret;
} repo_verify_info;

static int verify_user_repos_cb(char* fullPath, char* fileName, void* data) {
	repo_verify_info* info = (repo_verify_info*)data;
	
	info->ret |= verify_repo_structure(info->syspath, info->username, fileName);

	return 0;
}


int verify_user_repos(char* syspath, char* username) {
	repo_verify_info info = {
		.syspath = syspath,
		.username = username,
		.ret = 0,
	};
	
	char* repo_dir = path_join(syspath, "users", username, "repos"); 
	recurse_dirs(repo_dir, verify_user_repos_cb, &info, 0, FSU_EXCLUDE_FILES | FSU_INCLUDE_DIRS | FSU_NO_FOLLOW_SYMLINKS | FSU_EXCLUDE_HIDDEN);
	free(repo_dir);
	
	return info.ret;
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
	
	verify_repo_structure(syspath, username, reponame);
	
	return 0;
}



