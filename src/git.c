#include <stdlib.h>
#include <string.h>


#include "git.h"
#include "sys.h"





void free_git_repo(git_repo* gr) {
	free(gr->repo_name);
	free(gr->owner);
	free(gr->abs_base_path);
	free(gr->abs_src_path);
	free(gr->abs_meta_path);
	free(gr->abs_pulls_path);
	free(gr->abs_wiki_path);
	free(gr->abs_issues_path);
}



char* git_count_commits(git_repo* gr, char* branch) {
	//	git rev-list --count <branch-name>
	return systemf("git --work-tree=%s --git-dir=%s/.git/ --no-pager rev-list --count %s",  gr->abs_src_path, gr->abs_src_path, branch);
}

long git_count_commits_int(git_repo* gr, char* branch) {
	
	char* s = git_count_commits(gr, branch);
	long l = strtol(s, NULL, 10);
	free(s);
	
	return l;
}


char* git_count_branches(git_repo* gr) {
	return sprintfdup("%ld", git_count_branches_int(gr));
}


long git_count_branches_int(git_repo* gr) {
	char* branches = git_get_local_branches(gr);
	
	long count = 0;
	for(char* s = branches; *s; s++) {
		if(*s == '\n') count++;
	}
	
	free(branches);
	
	return count;
}

char* git_get_local_branches(git_repo* gr) {
	return systemf("git --work-tree=%s --git-dir=%s/.git/ --no-pager branch -l",  gr->abs_src_path, gr->abs_src_path);
}


// just the raw text
char* git_get_file(git_repo* gr, char* branch, char* path) {
	return systemf("git --work-tree=%s --git-dir=%s/.git/ --no-pager show %s:%s",  gr->abs_src_path, gr->abs_src_path, branch, path);
}



void git_repo_parse_request(git_repo* gr, strlist* uri_parts) {
	
	
	/*
	// extract username
//	if(0 && rm->enable_users) {
	if(sl->len >= 1) {
		gr->owner = strdup(uri_parts->entries[0]);
	}
	
	if(sl->len >= 2) {
		gr->repo_name = strdup(uri_parts->entries[1]);
	}
	
*/
	
	
	
	
}



