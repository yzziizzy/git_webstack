#include <stdio.h>


#include "../git_browse.h"
#include "../cproc.h"
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>



void do_site_homepage(request_info* ri, scgi_request* req, connection_t* con) {
	char* resp = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
		
	repo_meta* rm = ri->rm;
		
	cw(resp);
	html_header(con);
	
	cw("<h1>Users</h1>");
	
	cw("<table class=\"dirlisting\">");
	char* cmd = sprintfdup("find %s/users/ -maxdepth 1 -type d", rm->path);
	char* str = sysstring(cmd);
	
	char** dirs = str_split(str, "\r\n");
	
	free(cmd);
	free(str);
	
	for(char** s = dirs; *s; s++) {
		
		char* w = strrchr(*s, '/');
		
		if(*w == '/') w++;
		if(strlen(w) == 1) continue;
		
		
		cw("<tr>");
		
		cw("<td><a href=\"/");
		cw(w);
		cw("\">");
		cw(w);
		cw("</a></td>");
		
		cw("</tr>");
	}
	cw("</table>");
	
	html_footer(con);
	
}






