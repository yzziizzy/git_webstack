#ifndef __GWS__git_h__
#define __GWS__git_h__

#include "sti/sti.h"
#include "strlist.h"




typedef struct {
	char* repo_name;
	char* owner;

	char* abs_base_path;	
	char* abs_src_path;
	char* abs_meta_path;
	char* abs_pulls_path;
	char* abs_wiki_path;
	char* abs_issues_path;
} git_repo;


void free_git_repo(git_repo* gr);


#endif // __GWS__git_h__
