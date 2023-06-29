#include <stdio.h>


#include "git_browse.h"
#include "cproc.h"
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>

#include "uri.h"
#include "sys.h"
#include "html.h"





void do_project_index(repo_meta* rm, scgi_request* req, connection_t* con) {
	char* resp = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
		
	cw(resp);
	html_header(con);
	
	cw("<table class=\"dirlisting\">");
	char* cmd = sprintfdup("find %s/ -maxdepth 1 -type d", rm->path);
	char* str = sysstring(cmd);
	free(cmd);
	
	char** dirs = str_split(str, "\r\n");
	free(str);
	
	for(char** s = dirs; *s; s++) {
		
		char* w = strrchr(*s, '/');
		if(strlen(w) == 1) continue;
		
		
		cw("<tr>");
		
		cw("<td><a href=\"");
		cw(w);
		cw("\">");
		cw(w);
		cw("</a></td>");
		
		cw("</tr>");
	}
	cw("</table>");
	
	html_footer(con);
	
}

void do_folder(request_info* ri, scgi_request* req, connection_t* con) {

	char* cmd = sprintfdup("git --work-tree=%s --git-dir=%s/.git/ ls-tree --name-only HEAD ./%s/", ri->abs_project_path, ri->abs_project_path, ri->branch, ri->rel_file_path);
	char* str = sysstring(cmd);
	printf("gitlscmd: '%s'\n", str);
	free(cmd);
	
//	printf("'%s'\n", str);
	char** files = str_split(str, "\r\n");
	
	char* cmd_fmt = "git --work-tree=%s --git-dir=%s/.git/  --no-pager log -1 --pretty=format:%%H%%n%%cn%%n%%ce%%n%%ci%%n%%cr%%n%%s%%n -- %s";
	
	
	
	char* resp = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
		
	cw(resp);
	html_header(con);
	
	cw("<table class=\"dirlisting\">");
	
	char* a = strdup("");
	
	cw("<tr><th colspan=4>/");
	for(int i = 0; i < ri->file_path_parts->len; i++) {
		if(strlen(ri->file_path_parts->entries[i]) == 0) continue;
		char* b = path_join(a, ri->file_path_parts->entries[i]);
		cw("<a href=\"");
		cw(b);
		cw("\">");
		cw(ri->file_path_parts->entries[i]);
		cw("</a>");
		
		free(a);
		a = b;
		
		if(i < ri->file_path_parts->len - 1) {
			cw("/");
		}
	}
	cw("</th></tr>");
	
	free(a);
	
	for(char** s = files; *s; s++) {
		char* cmd = sprintfdup(cmd_fmt, ri->abs_project_path, ri->abs_project_path, *s);
		char* res = sysstring(cmd);
		printf("cmd: %s\n", cmd);
		free(cmd);
		
		char** data = str_split(res, "\r\n");
		cw("<tr>");
		
		cw("<td><a href=\"/");
		cw(ri->file_path_parts->entries[1]); cw("/"); cw(*s);
		cw("\">");
		
		char* w = strrchr(*s, '/');
		cw(w ? w + 1 : *s);
		cw("</a></td>");
		
			cw("<td>");
			cw(data[5]);
			cw("</td>");
			
			cw("<td>");
			cw(data[4]);
			cw("</td>");
		
		cw("</tr>");
		
		free(res);
		free_strpp(data);
	}
	cw("</table>");
	
	html_footer(con);

	free(str);
	free_strpp(files);

}

void do_file(request_info* ri, scgi_request* req, connection_t* con) {


	char* cmd = sprintfdup("git --work-tree=%s --git-dir=%s/.git/  --no-pager blame -lc %s", ri->abs_project_path, ri->abs_project_path, ri->rel_file_path);
	char* str = sysstring(cmd);
	free(cmd);
	
//	printf("'%s'\n", str);
	char** files = str_split(str, "\r\n");

	char* resp = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
	
	cw(resp);
	html_header(con);
	
	cw("<table class=\"codelisting lang-c\">");
	
	char* a = strdup("");
	
	cw("<tr><th colspan=4>/");
	for(int i = 0; i < ri->file_path_parts->len; i++) {
		if(strlen(ri->file_path_parts->entries[i]) == 0) continue;
		char* b = path_join(a, ri->file_path_parts->entries[i]);
		cw("<a href=\"");
		cw(b);
		cw("\">");
		cw(ri->file_path_parts->entries[i]);
		cw("</a>");
		
		free(a);
		a = b;
		
		if(i < ri->file_path_parts->len - 1) {
			cw("/");
		}
	}
	cw("</th></tr>");
	
	free(a);
	
	for(char** s = files; *s; s++) {

		char* hash_end = strchr(*s, '\t');
		char* hash = *s;
		
		char* user = hash_end + strspn(hash_end, "(\t ");
		char* user_end = strchr(user, '\t');
		
		char* timestamp = strpbrk(user_end, "\t") + 1;
		char* timestamp_end = strchr(timestamp, '\t');
		
		char* line = strpbrk(timestamp_end, "\t") + 1;
		char* line_end = strchr(line, ')');
		
		char* text = line_end + 1;
		
		cw("<tr hash=\"");
		cnw(hash, hash_end - hash);
		cw("\" user=\"");
		cnw(user, user_end - user);
		cw("\" timestamp=\"");
		cnw(timestamp, timestamp_end - timestamp);
		cw("\" line=\"");
		cnw(line, line_end - line);
		cw("\">\n");
		
		
		cw("<td>");
		cnw(user, user_end - user);
		cw("</td>\n");
		
		cw("<td id=\"L");
		cnw(line, line_end - line);
		cw("\"><a href=\"#L");
		cnw(line, line_end - line);
		cw("\">");
		cnw(line, line_end - line);
		cw("</a></td>\n");
		
		cw("<td><c>");
		char* textsafe = html_encode(text, -1);
		cw(text);
		free(textsafe);
		cw("</c></td>\n");
		
		
		cw("</tr>\n");
		
	}
	cw("</table>");
	
	
	html_footer(con);

	free(str);
	free_strpp(files);

}


int is_valid_branch(request_info* ri) {
	printf("is_valid_branch nyi\n");
	
	return 0 == strcmp(ri->branch, "master");
}


void extract_file_path(request_info* ri, strlist* tmp_uri_parts) {
	ri->file_path_parts = strlist_new();
	
	// check that it's a valid path the whole way
	for(int i = 0; i < tmp_uri_parts->len; i++) {
		char* p = tmp_uri_parts->entries[i]; 
		
		char type = get_file_type(p);
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
	
	}
		
	ri->rel_file_path = join_str_list(ri->file_path_parts->entries, "/");
	ri->abs_file_path = path_join(ri->abs_project_path, ri->rel_file_path);
}


void render_src_view(request_info* ri, scgi_request* req, connection_t* con) {
	
	
	if(ri->leaf_type == 'd') {
		do_folder(ri, req, con); 
		return;
	}
	else if(ri->leaf_type == 'f') {
		do_file(ri, req, con);
		return;
	}
	else {
		printf("bad leaf type handler nyi\n");
		cw("Status: 404\r\n\r\n");
		return;
	}

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
		cw("Status: 404\r\n\r\n");
		printf("site homepage nyi\n");
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
			printf("user profile homepage nyi\n");
			cw("Status: 404\r\n\r\n");
			return;
		}
		
		// extract and validate project name
		ri.project = strlist_shift(tmp_uri_parts);
		ri.abs_project_path = path_join(ri.abs_user_path, "repos", ri.project);
		
		if(!is_dir(ri.abs_project_path)) {
			printf("project '%s'/'%s' not found\n", ri.username, ri.project);
			cw("Status: 404\r\n\r\n");
			return;
		}
		
		// check for project homepage
		if(tmp_uri_parts->len == 0) {
			printf("project homepage nyi\n");
			cw("Status: 404\r\n\r\n");
			return;
		}
		
		// extract category (src, wiki, issues, etc.)
		ri.category = strlist_shift(tmp_uri_parts);
		if(0 == strcmp(ri.category, "src")) {
			
			// check for branch list page
			if(tmp_uri_parts->len == 0) {
				printf("branch list nyi\n");
				cw("Status: 404\r\n\r\n");
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
			
			render_src_view(&ri, req, con);
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
	
	
	// extract project (repo) name
	if(1 || rm->enable_projects) {
		
		ri.project = strlist_shift(tmp_uri_parts);
	
		
		if(ri.project == NULL || (strlen(ri.project) == 0)) {
			do_project_index(rm, req, con);
			
			// TODO proper cleanup
			strlist_free(ri.uri_parts, 1);
			return;
		}
	}
		
	// extract the category
	if(0 == strcmp(rm->source_uri_part, tmp_uri_parts->entries[0])) {
		ri.request_type = RT_SourceView;
		free(strlist_shift(tmp_uri_parts));
	}
	else if(0 == strcmp(rm->pulls_uri_part, tmp_uri_parts->entries[0])) {
		ri.request_type = RT_PullRequests;
		free(strlist_shift(tmp_uri_parts));
	}
	else {
		ri.request_type = RT_ProjectPage;
	}
	
	
	// extract the branch, if its something that needs in
	if(ri.request_type == RT_SourceView) {
		ri.branch = strlist_shift(tmp_uri_parts);
		
		if(!ri.branch || strlen(ri.branch) == 0) {
			printf("ERR: no branch found\n");
			
			do_project_index(rm, req, con);
			
			// TODO proper cleanup
			strlist_free(ri.uri_parts, 1);
			return;
			
		}
	}
	
	


//
//	
//	if(strstr(uri, "..")) {
//		printf("'..' found in request uri\n");
//		cw("Status: 404\r\n\r\n");
//		return;
//	}
//	
	

	
			
}


