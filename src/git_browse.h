#ifndef __GWS__git_browse_h__
#define __GWS__git_browse_h__


#include "scgi.h"
#include "strlist.h"
#include "git.h"
#include "uri.h"
#include "sys.h"
#include "html.h"



typedef struct {
	char* path;

	char* static_asset_path;

	char enable_users;	
	char enable_projects;
	
	// http://domain.tld/user/project/[source_uri_part]/local/path/to/file.c
	char* source_uri_part; 
	char* pulls_uri_part; 
	char* wiki_uri_part; 
	char* issues_uri_part; 
	
	
	
} repo_meta;


typedef struct {
	strlist* components;
	char* fullpath; // dirty atm
	char* repo_path;
	char* rel_path;

} path_info;


#define REQUEST_TYPE_LIST \
	X(SourceView) \
	X(PullRequests) \
	X(UserProfile) \
	X(ProjectPage) \
	X(RootPage) \

enum {
#define X(a, ...) RT_##a,
	REQUEST_TYPE_LIST
#undef X
	RT_MAX_VALUE,
};


typedef struct request_info {
	
	strlist* uri_parts;

	int request_type;

	char* username;
	char* project;
	char* category; // src,meta,wiki,issues,pulls,etc
	char* branch;
	
	char leaf_type; // f,d for file directory. Used with SourceView
	
	strlist* file_path_parts;
	char* abs_user_path;
	char* abs_project_path;
	char* abs_src_path;
	char* abs_file_path;
	char* rel_file_path;

	git_repo gr;
	git_path gp;

	path_info* pi;
	repo_meta* rm;
} request_info;





void git_browse_handler(void* user_data, scgi_request* req, connection_t* con);


void do_project_index(request_info* ri, scgi_request* req, connection_t* con);
void do_folder(request_info* ri, scgi_request* req, connection_t* con);
void do_file(request_info* ri, scgi_request* req, connection_t* con);
void do_src_view(request_info* ri, scgi_request* req, connection_t* con);
void render_folder(git_repo* gr, git_path* gp, scgi_request* req, connection_t* con);
void do_project_homepage(request_info* ri, scgi_request* req, connection_t* con);

void do_site_homepage(repo_meta* rm, scgi_request* req, connection_t* con);

#endif // __GWS__git_browse_h__
