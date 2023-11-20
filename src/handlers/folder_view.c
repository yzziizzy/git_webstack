#include <stdio.h>



#include "../git_browse.h"
#include "../cproc.h"
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>



void do_src_view(request_info* ri, scgi_request* req, connection_t* con) {
	
	
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




void do_folder(request_info* ri, scgi_request* req, connection_t* con) {
	printf("rendering folder\n");

	char* resp = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
		
	cw(resp);
	html_header(con);
	html_repo_header(con, &ri->gr);
	
	render_folder(&ri->gr, &ri->gp, req, con);
	
	html_footer(con);



}



void render_folder(git_repo* gr, git_path* gp, scgi_request* req, connection_t* con) {

	char* cmd = sprintfdup("git --work-tree=%s --git-dir=%s/ ls-tree --name-only %s ./%s/", 
		gr->abs_src_path, gr->abs_src_path, gp->branch, gp->rel_file_path);
	char* str = sysstring(cmd);
	printf("gitlscmd: '%s'\n", cmd);
	free(cmd);
	
//	printf("'%s'\n", str);
	char** files = str_split(str, "\r\n");
	
	char* cmd_fmt = "git --work-tree=%s --git-dir=%s/  --no-pager log -1 --pretty=format:%%H%%n%%cn%%n%%ce%%n%%ci%%n%%cr%%n%%s%%n -- %s";

	cw("<table class=\"dirlisting\">");
	
	char* a = strdup("");
	
	cw("<tr><th colspan=4>/");
	for(int i = 0; i < gp->file_path_parts->len; i++) {
		if(strlen(gp->file_path_parts->entries[i]) == 0) continue;
		char* b = path_join(a, gp->file_path_parts->entries[i]);
		cw("<a href=\"/", gr->owner, "/", gr->repo_name, "/src/", gp->branch);
		cw(b);
		cw("\">");
		cw(gp->file_path_parts->entries[i]);
		cw("</a>");
		
		free(a);
		a = b;
		
		if(i < gp->file_path_parts->len - 1) {
			cw("/");
		}
	}
	cw("</th></tr>");
	
	free(a);
	
	for(char** s = files; *s; s++) {
		char* cmd = sprintfdup(cmd_fmt, gr->abs_src_path, gr->abs_src_path, *s);
		char* res = sysstring(cmd);
		printf("cmd: %s\n", cmd);
		free(cmd);
		
		char** data = str_split(res, "\r\n");
		cw("<tr>");
		
		cw("<td><a href=\"/");
		cw(gr->owner); cw("/"); cw(gr->repo_name); cw("/src/"); cw(gp->branch); cw("/"); cw(*s); // BUG?
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
	
	
	free(str);
	free_strpp(files);
}



