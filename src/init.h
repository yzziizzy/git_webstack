#ifndef __GWS__init_h__
#define __GWS__init_h__



void initialize_path_for_system(char* path, char clobber);
void initialize_user(char* syspath, char* username, char* email, char clobber);
int initialize_repo(char* syspath, char* username, char* reponame);
int create_issue(git_repo* gr, char* issue_username, char* issue_file_path);
int issue_add_comment(git_issue* gi, char* comment_username, char* comment_file_path);

int verify_all_users(char* syspath);
int verify_user(char* syspath, char* username);
int verify_user_repos(char* syspath, char* username);
int verify_repo_structure(char* syspath, char* username, char* reponame);

void write_system_version(char* syspath, int version);
int get_system_version(char* syspath);



#endif // __GWS__init_h__
