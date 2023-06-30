#include <stdio.h>


#include "../git_browse.h"
#include "../cproc.h"
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>



void do_site_homepage(repo_meta* rm, scgi_request* req, connection_t* con) {
	char* resp = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
		
	cw(resp);
	html_header(con);
	
	cw("<h1>Users</h1>");
	
	cw("<table class=\"dirlisting\">");
	char* cmd = sprintfdup("find %s/users/ -maxdepth 1 -type d", rm->path);
	char* str = sysstring(cmd);
	free(cmd);
	
	char** dirs = str_split(str, "\r\n");
	free(str);
	
	for(char** s = dirs; *s; s++) {
		
		char* w = strrchr(*s, '/');
		
		if(*w == '/') w++;
		if(strlen(w) == 1) continue;
		
		
		cw("<tr>");
		
		cw("<td><a href=\"/u/");
		cw(w);
		cw("\">");
		cw(w);
		cw("</a></td>");
		
		cw("</tr>");
	}
	cw("</table>");
	
	html_footer(con);
	
}






