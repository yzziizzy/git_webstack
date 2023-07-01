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


typedef struct {
	char type; // f,d

	char* branch;
	char* filename; // name of the final component of the path
	
	strlist* file_path_parts;

	char* abs_file_path;
	char* rel_file_path;
} git_path;


typedef struct {
	char* creator;
	char* repo_owner;
	char* repo_name;
	int user_issue_num;
	char open;
	
	char* folder_name;
	
	char* abs_path;
} git_issue;



void free_git_repo(git_repo* gr);
void free_git_issue(git_issue* gi);


int git_repo_init_short(git_repo* gr, char* repos_path, char* target_repo);
int git_issue_init_short(git_issue* gi, char* repos_path, char* target_issue);

char* git_count_commits(git_repo* gr, char* branch);
long git_count_commits_int(git_repo* gr, char* branch);

char* git_count_branches(git_repo* gr);
long git_count_branches_int(git_repo* gr);
char* git_get_local_branches(git_repo* gr);

char* git_get_file(git_repo* gr, char* branch, char* path);

#endif // __GWS__git_h__
