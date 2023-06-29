#include <stdlib.h>
#include <string.h>


#include "git.h"





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



