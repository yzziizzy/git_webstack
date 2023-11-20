#include <stdio.h>


#include "../git_browse.h"
#include "../cproc.h"
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>




void do_project_index(request_info* ri, scgi_request* req, connection_t* con) {
	char* resp = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
		
	cw(resp);
	html_header(con);
	
	cw("<table class=\"dirlisting\">");
	char* cmd = sprintfdup("find %s/repos/ -maxdepth 1 -type d", ri->abs_user_path);
	char* str = sysstring(cmd);
	free(cmd);
	
	char** dirs = str_split(str, "\r\n");
	free(str);
	
	for(char** s = dirs; *s; s++) {
		
		char* w = strrchr(*s, '/');
		if(strlen(w) == 1) continue;
		
		
		cw("<tr>");
		
		cw("<td><a href=\"/");
		cw(ri->username);
		cw(w);
		cw("\">");
		cw(w);
		cw("</a></td>");
		
		cw("</tr>");
	}
	cw("</table>");
	
	html_footer(con);
	
}





