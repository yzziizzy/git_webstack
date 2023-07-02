#include <stdio.h>


#include "../git_browse.h"
#include "../cproc.h"
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>



void do_project_issues(request_info* ri, scgi_request* req, connection_t* con) {
	
	memtricks_set_shitty_arena();
	
	cw("Status: 200 OK\r\nContent-Type: text/html\r\n\r\n");
	
	html_header(con);
	html_repo_header(con, &ri->gr);
	
	cw("<h1>Issues</h1>");
	
	cw("<table class=\"issuelisting\">");
	
	git_issue_list* list = git_open_issues(&ri->gr);
	
	VEC_EACH(&list->issues, i, issue) {
		
		cw("<tr>");
		cw("<td><a href=\"/u/", issue->repo_owner, "/", issue->repo_name, "/issues/", issue->creator, "/");
		char buf[40];
		snprintf(buf, 40, "%d", issue->user_issue_num);
		cw(buf, "\">");
		cw(issue->creator, "#", buf);
		cw("</a></td>");
		
		cw("</tr>");
	}
	cw("</table>");
	
	html_footer(con);
	
	memtricks_shitty_arena_exit();
}










