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



void verify_file_path(request_info* ri, git_repo* gr, char* branch) {
	
	ri->leaf_type = 'd'; // project root dir by default
	
	char* base = strdup("");
	// check that it's a valid path the whole way
	for(int i = 0; i < ri->file_path_parts->len; i++) {
		char* p = ri->file_path_parts->entries[i]; 
		char* path = path_join(base, p);
		
		printf("checking path '%s'\n", path);
		char type = git_file_type(gr, branch, path);
		if(type == 'd') {
			printf("is dir\n");
			ri->leaf_type = 'd';
		}
		else if(type == 'f') {
			printf("is file\n");
			ri->leaf_type = 'f';
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








static void insert_path(URIPart* p, char** s, page_handler_fn fn) {
	if(*s == NULL) return;
	
	VEC_EACH(&p->kids, i, k) {
		if(strcmp(*s, k->string)) continue;
		
		insert_path(k, s + 1, fn);
		return;
	}
	
	// insert new child
	URIPart* k = calloc(1, sizeof(*k));
	k->string = strdup(*s);
	k->fn = fn;
	
	VEC_PUSH(&p->kids, k);
}


URIPart* init_uri_tree(struct uri_pair* pairs) {
	URIPart* root = calloc(1, sizeof(*root));
	
	
	for(struct uri_pair* p = pairs; p->s; p++) {
		
		char** partlist = strsplit(p->s, '/', NULL);
		
		insert_path(root, partlist, p->fn);
		
		// TODO: free partlist
		
	}
	
	// TODO: validate the paths:
	// %f must be last element
	// %'s must only be one letter long
	// no duplicate paths
	// no ambigious paths:
	//    only one % choice in each kids list
	// only one % of each type allowed per path
	// no invalid characters
	// no empty-string path sections
	
	return root;
}

static int uri_tree_sort_fn(URIPart** a, URIPart** b) {
	return strcmp((**b).string, (**a).string);
}

// % is conveniently before every reasonable, valid uri character
void sort_uri_tree(URIPart* p) {
	VEC_SORT(&p->kids, uri_tree_sort_fn);
	VEC_EACH(&p->kids, i, k) sort_uri_tree(k);
}

void print_uri_tree(URIPart* p, int indent) {
	
	for(int i = 0; i < indent; i++) printf("  ");
	
	printf("%s\n", p->string ? p->string : "<root>");
	VEC_EACH(&p->kids, i, k) {
		print_uri_tree(k, indent + 1);
	}
}

page_handler_fn match_uri(URIPart* p, request_info* ri, int partIndex) {
	if(partIndex >= ri->uri_parts->len) return p->fn;
	
	char* part = ri->uri_parts->entries[partIndex];
	
	printf("part: %s \n", part);
	
	VEC_EACH(&p->kids, i, k) {
		printf(" trying %s\n", k->string);
		if(!strcmp(part, k->string)) {
			return match_uri(k, ri, partIndex + 1);
		}
		
		if(k->string[0] == '%') {
			printf(" trying %% \n");
			if(k->string[1] == 'u') {
				ri->username = strdup(part); // TODO: should be duped?
				return match_uri(k, ri, partIndex + 1);
			}
			else if(k->string[1] == 'r') {
				ri->project = strdup(part); // TODO: should be duped?
				return match_uri(k, ri, partIndex + 1);
			}
			else if(k->string[1] == 'b') {
				ri->branch = strdup(part); // TODO: should be duped?
				return match_uri(k, ri, partIndex + 1);
			}
			else if(k->string[1] == 'i') {
				ri->issue = strdup(part); // TODO: should be duped?
				return match_uri(k, ri, partIndex + 1);
			}
			else if(k->string[1] == 'f') {
				
				// grab the entire rest of the uri as the path and return early
				ri->file_path_parts = strlist_clone_from(ri->uri_parts, partIndex);
				return k->fn;
			}
		}
	}
	
	return NULL;
}




void git_browse_handler(void* user_data, scgi_request* req, connection_t* con) {
	repo_meta* rm = user_data;

	printf("path: '%s'\n", rm->path);
	
	memtricks_set_shitty_arena();
	
	char* uri = NULL;
	VEC_EACHP(&req->headers, i, h) {
		if(0 == strcmp(h->key, "DOCUMENT_URI")) {
			uri = h->value;
		}
	}
	
//	printf("'%s'\n", uri);
	
	// ignore requests for the favicon
	if(0 == strcmp(uri, "/favicon.ico")) {
		char* resp = "Status: 404\r\n\r\n";
		cw(resp);
		goto CLEANUP;
	}
	
	// handle requests for static files
	if(0 == strncmp(uri, "/_/", 3)) {
		
		if(strstr(uri, "..")) {
			printf("'..' found in static asset path request\n");
			cw("Status: 404\r\n\r\n");
			goto CLEANUP;
		}
		
		char* testpath = path_join(rm->static_asset_path, uri + 3);
		
		size_t slen = 0;
		char* src = read_whole_file(testpath, &slen);
		if(!src) {
			printf("no such static asset '%s'\n", uri);
			cw("Status: 404\r\n\r\n");
			goto CLEANUP;
		}
		
		cw("Status: 200\r\n\r\n");
		cnw(src, slen - 1);
		goto CLEANUP;
	}
	
	
	request_info ri = {0};
	ri.rm = rm;
	
	ri.uri_parts = strlist_new();
	
	parse_uri(uri, ri.uri_parts);
	
	strlist* tmp_uri_parts = strlist_clone(ri.uri_parts);
	
	// URL formats:
	/*
	
	http://domain.tld/                        -- site homepage
	http://domain.tld/_/                      -- static assets
	http://domain.tld/[username]            -- user profile 
	http://domain.tld/u/[username]/           -- user meta
	http://domain.tld/u/[username]/newrepo    -- new repo page
	http://domain.tld/[username]/[repo]/      -- repo homepage
	http://domain.tld/[username]/[repo]/src   -- branch list
	http://domain.tld/[username]/[repo]/src/[branch]                     -- root directory listing of the branch/commit
	http://domain.tld/[username]/[repo]/src/[branch]/[/path/to/file.c]   -- source view of file
	http://domain.tld/[username]/[repo]/status  -- repo meta
	http://domain.tld/[username]/[repo]/settings  -- repo meta
	
	
	%u username
	%r repo name
	%b branch
	%f file path
	
	*/
	

	
	// check for homepage
	if(tmp_uri_parts->len == 0) {
		do_site_homepage(&ri, req, con);
		goto CLEANUP;
	}
	
	
	page_handler_fn fn = match_uri(rm->uri_tree, &ri, 0);
	printf("fn: %p\n", fn);
	
	if(!fn) {
		cw("Status: 404\r\n\r\n");
		cw("Page not found");
		goto CLEANUP;
	}
	
	// TODO: validate all uri-derived info
	
	if(ri.username) {
		ri.abs_user_path = path_join(rm->path, "users", ri.username);
		
		// validate username
		if(!is_dir(ri.abs_user_path)) {
			printf("user '%s' not found\n", ri.username);
			cw("Status: 404\r\n\r\n");
			goto CLEANUP;
		}
	}
	
	
	if(ri.project) {
		ri.abs_project_path = path_join(ri.abs_user_path, "repos", ri.project);
		ri.abs_src_path = path_join(ri.abs_project_path, "src.git");
	
		if(!is_dir(ri.abs_project_path)) {
			printf("project '%s'/'%s' not found\n", ri.username, ri.project);
			cw("Status: 404\r\n\r\n");
			goto CLEANUP;
		}
	
		ri.gr.repo_name = strdup(ri.project);
		ri.gr.owner = strdup(ri.username);
		ri.gr.abs_base_path = strdup(ri.abs_project_path);
		ri.gr.abs_src_path = strdup(ri.abs_src_path);
		ri.gr.abs_meta_path = path_join(ri.abs_project_path, "meta");
		ri.gr.abs_wiki_path = path_join(ri.abs_project_path, "wiki");
		ri.gr.abs_pulls_path = path_join(ri.abs_project_path, "pulls");
		ri.gr.abs_issues_path = path_join(ri.abs_project_path, "issues");
	}
	
	if(ri.branch) {
		if(!is_valid_branch(&ri)) {
			printf("invalid branch handler nyi (%s/%s/%s)\n", ri.username, ri.project, ri.branch);
			cw("Status: 404\r\n\r\n");
			goto CLEANUP;
		}
	}
	
	
	if(ri.file_path_parts && ri.file_path_parts->len) {
		
		// extract file path
		verify_file_path(&ri, &ri.gr, ri.branch);
		
		if(ri.leaf_type == 0) {
			printf("File not found (%s/%s/%s)\n", ri.username, ri.project, ri.branch);
			cw("Status: 404\r\n\r\n");
			goto CLEANUP;
		
		}
		
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
	}
	
	fn(&ri, req, con);
	goto CLEANUP;
	
	
	/*
	
	
	
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
			goto CLEANUP;
		}
		
		
		// check for user profile homepage
		if(tmp_uri_parts->len == 0) {
//			printf("user profile homepage nyi\n");
			do_project_index(&ri, req, con);
			goto CLEANUP;
		}
		
		// extract and validate project name
		ri.project = strlist_shift(tmp_uri_parts);
		ri.abs_project_path = path_join(ri.abs_user_path, "repos", ri.project);
		ri.abs_src_path = path_join(ri.abs_project_path, "src.git");
		
		if(!is_dir(ri.abs_project_path)) {
			printf("project '%s'/'%s' not found\n", ri.username, ri.project);
			cw("Status: 404\r\n\r\n");
			goto CLEANUP;
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
			goto CLEANUP;
		}
		
		// extract category (src, wiki, issues, etc.)
		ri.category = strlist_shift(tmp_uri_parts);
		if(0 == strcmp(ri.category, "src")) {
			
			// check for branch list page
			if(tmp_uri_parts->len == 0) {
				printf("branch list nyi\n");
				http302(con, "/u/", ri.username, "/", ri.project, "/src/master");
				goto CLEANUP;
			}
			
			// extract and validate branch
			ri.branch = strlist_shift(tmp_uri_parts);
			
			if(!is_valid_branch(&ri)) {
				printf("invalid branch handler nyi (%s/%s/%s)\n", ri.username, ri.project, ri.branch);
				cw("Status: 404\r\n\r\n");
				goto CLEANUP;
			}
		
			// extract file path
			extract_file_path(&ri, &ri.gr, ri.branch, tmp_uri_parts);
			
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
			goto CLEANUP;
		}
		else if(0 == strcmp(ri.category, "issues")) {
		
			// check for issue list page
			if(tmp_uri_parts->len == 0) {
				do_project_issues(&ri, req, con);
				goto CLEANUP;
			}
			
			ri.gi.creator = strlist_shift(tmp_uri_parts);
			
			if(tmp_uri_parts->len == 0) {
				printf("issue-by-creator list nyi\n");
				cw("Status: 404\r\n\r\n");
				goto CLEANUP;
			}
			
			ri.gi.user_issue_num_str = strlist_shift(tmp_uri_parts);
			ri.gi.user_issue_num = strtol(ri.gi.user_issue_num_str, NULL, 10);
			
			do_issue(&ri, &ri.gi, req, con);
			goto CLEANUP;
		}
		else {
			printf("category '%s' nyi\n", ri.category);
			cw("Status: 404\r\n\r\n");
			goto CLEANUP;
		}
		
		
		
	}
	
	*/

//
//	
//	if(strstr(uri, "..")) {
//		printf("'..' found in request uri\n");
//		cw("Status: 404\r\n\r\n");
//		return;
//	}
//	
CLEANUP:
	memtricks_shitty_arena_exit();

	
			
}


