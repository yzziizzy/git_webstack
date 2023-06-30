#include <stdio.h>


#include "git_browse.h"
#include "cproc.h"
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>

#include "uri.h"
#include "sys.h"
#include "html.h"






int is_valid_branch(request_info* ri) {
	printf("is_valid_branch nyi\n");
	
	return 0 == strcmp(ri->branch, "master");
}


void extract_file_path(request_info* ri, strlist* tmp_uri_parts) {
	ri->file_path_parts = strlist_new();
	
	ri->leaf_type = 'd'; // project root dir by default
	
	char* base = strdup(ri->abs_src_path);
	// check that it's a valid path the whole way
	for(int i = 0; i < tmp_uri_parts->len; i++) {
		char* p = tmp_uri_parts->entries[i]; 
		char* path = path_join(base, p);
		
		printf("checking path '%s'\n", path);
		char type = get_file_type(path);
		if(type == 'd') {
			printf("is dir\n");
			ri->leaf_type = 'd';
			strlist_push(ri->file_path_parts, strdup(p));
		}
		else if(type == 'f') {
			printf("is file\n");
			ri->leaf_type = 'f';
			strlist_push(ri->file_path_parts, strdup(p));
			break;
		}
		else {
			// TODO: 404
			ri->leaf_type = 0;
			break;
		}
	
		free(base);
		base = path;
	}
	
	free(base);
	
	ri->rel_file_path = join_str_list(ri->file_path_parts->entries, "/");
	ri->abs_file_path = path_join(ri->abs_project_path, ri->rel_file_path);
}




void git_browse_handler(void* user_data, scgi_request* req, connection_t* con) {
	repo_meta* rm = user_data;

	printf("path: '%s'\n", rm->path);
	
	
	char* uri = NULL;
	VEC_EACHP(&req->headers, i, h) {
		if(0 == strcmp(h->key, "DOCUMENT_URI")) {
			uri = h->value;
		}
	}
	
	printf("'%s'\n", uri);
	
	// ignore requests for the favicon
	if(0 == strcmp(uri, "/favicon.ico")) {
		char* resp = "Status: 404\r\n\r\n";
		cw(resp);
		return;
	}
	
	// handle requests for static files
	if(0 == strncmp(uri, "/_/", 3)) {
		
		if(strstr(uri, "..")) {
			printf("'..' found in static asset path request\n");
			cw("Status: 404\r\n\r\n");
			return;
		}
		
		char* testpath = path_join(rm->static_asset_path, uri + 3);
		
		size_t slen = 0;
		char* src = read_whole_file(testpath, &slen);
		if(!src) {
			printf("no such static asset '%s'\n", uri);
			cw("Status: 404\r\n\r\n");
			return;
		}
		
		cw("Status: 200\r\n\r\n");
		cnw(src, slen - 1);
		return;
	}
	
	
	request_info ri = {0};
	
	
	ri.uri_parts = strlist_new();
	
	parse_uri(uri, ri.uri_parts);
	
	strlist* tmp_uri_parts = strlist_clone(ri.uri_parts);
	
	// URL formats:
	/*
	
	http://domain.tld/                        -- site homepage
	http://domain.tld/_/                      -- static assets
	http://domain.tld/[username]              -- user profile 
	http://domain.tld/u/[username]/           -- user meta
	http://domain.tld/[username]/[repo]/      -- repo homepage
	http://domain.tld/[username]/[repo]/src   -- branch list
	http://domain.tld/[username]/[repo]/src/[branch]                     -- root directory listing of the branch/commit
	http://domain.tld/[username]/[repo]/src/[branch]/[/path/to/file.c]   -- source view of file
	http://domain.tld/[username]/[repo]/status  -- repo meta
	http://domain.tld/[username]/[repo]/settings  -- repo meta
	
	
	
	*/
	
	
	// check for homepage
	if(tmp_uri_parts->len == 0) {
		do_site_homepage(rm, req, con);
		return;
	}
	
	// check for /u/
	if(0 == strcmp(tmp_uri_parts->entries[0], "u")) {
		free(strlist_shift(tmp_uri_parts));
		
		// extract username
		ri.username = strlist_shift(tmp_uri_parts);
		ri.abs_user_path = path_join(rm->path, "users", ri.username);
		
		// validate username
		if(!is_dir(ri.abs_user_path)) {
			printf("user '%s' not found\n", ri.username);
			cw("Status: 404\r\n\r\n");
			return;
		}
		
		
		// check for user profile homepage
		if(tmp_uri_parts->len == 0) {
//			printf("user profile homepage nyi\n");
			do_project_index(&ri, req, con);
			return;
		}
		
		// extract and validate project name
		ri.project = strlist_shift(tmp_uri_parts);
		ri.abs_project_path = path_join(ri.abs_user_path, "repos", ri.project);
		ri.abs_src_path = path_join(ri.abs_project_path, "src");
		
		if(!is_dir(ri.abs_project_path)) {
			printf("project '%s'/'%s' not found\n", ri.username, ri.project);
			cw("Status: 404\r\n\r\n");
			return;
		}
		
		ri.gr.repo_name = strdup(ri.project);
		ri.gr.owner = strdup(ri.username);
		ri.gr.abs_base_path = strdup(ri.abs_project_path);
		ri.gr.abs_src_path = strdup(ri.abs_src_path);
		ri.gr.abs_meta_path = path_join(ri.abs_project_path, "meta");
		ri.gr.abs_wiki_path = path_join(ri.abs_project_path, "wiki");
		ri.gr.abs_pulls_path = path_join(ri.abs_project_path, "pulls");
		ri.gr.abs_issues_path = path_join(ri.abs_project_path, "issues");
		
		// check for project homepage
		if(tmp_uri_parts->len == 0) {
			do_project_homepage(&ri, req, con);
			return;
		}
		
		// extract category (src, wiki, issues, etc.)
		ri.category = strlist_shift(tmp_uri_parts);
		if(0 == strcmp(ri.category, "src")) {
			
			// check for branch list page
			if(tmp_uri_parts->len == 0) {
				printf("branch list nyi\n");
				http302(con, "/u/", ri.username, "/", ri.project, "/src/master");
				return;
			}
			
			// extract and validate branch
			ri.branch = strlist_shift(tmp_uri_parts);
			
			if(!is_valid_branch(&ri)) {
				printf("invalid branch handler nyi (%s/%s/%s)\n", ri.username, ri.project, ri.branch);
				cw("Status: 404\r\n\r\n");
				return;
			}
		
			// extract file path
			extract_file_path(&ri, tmp_uri_parts);
			
			ri.gp.branch = strdup(ri.branch);
			if(ri.file_path_parts->len) {
				ri.gp.filename = strdup(ri.file_path_parts->entries[ri.file_path_parts->len - 1]);
			}
			else {
				ri.gp.filename = strdup("");
			}
			ri.gp.type = ri.leaf_type;
			ri.gp.file_path_parts = strlist_clone(ri.file_path_parts);
			ri.gp.abs_file_path = strdup(ri.abs_file_path);
			ri.gp.rel_file_path = strdup(ri.rel_file_path);
			
			do_src_view(&ri, req, con);
			return;
		}
		else {
			printf("category '%s' nyi\n", ri.category);
			cw("Status: 404\r\n\r\n");
			return;
		}
		
		
		
	}
	
	printf("non-/u/ urls nyi\n");
	cw("Status: 404\r\n\r\n");
	return;
	
	

//
//	
//	if(strstr(uri, "..")) {
//		printf("'..' found in request uri\n");
//		cw("Status: 404\r\n\r\n");
//		return;
//	}
//	
	

	
			
}


