#include <stdio.h>


#include "../git_browse.h"
#include "../cproc.h"
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>



void do_project_homepage(request_info* ri, scgi_request* req, connection_t* con) {
	char* resp = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
		
	cw(resp);
	html_header(con);
	
	html_repo_header(con, &ri->gr);
	
	cw("<table class=\"meta-links\"><tr>");
	char* commits = git_count_commits(&ri->gr, "master"); 
	cw("<td><a href=\"/", ri->username, "/", ri->project, "/commits\">", commits, " commits</a></td>");
	free(commits);
	
	char* branches = git_count_branches(&ri->gr); 
	cw("<td><a href=\"/", ri->username, "/", ri->project, "/branches\">", branches, " branch");
	if(!(branches[0] == '1' && branches[1] == 0)) cw("es");
	cw("</a></td>");
	free(branches);
	
	char issues[30];
	long issue_cnt = git_count_open_issues(&ri->gr); 
	snprintf(issues, 30, "%ld", issue_cnt);
	cw("<td><a href=\"/", ri->username, "/", ri->project, "/issues\">", issues, " issue");
	if(issue_cnt != 1) cw("s");
	cw("</a></td>");
	
	cw("</tr></table>");
	
	git_path gp;
	gp.type = 'd';
	gp.branch = "master";
	gp.file_path_parts = strlist_new();
	gp.filename = "";
	gp.abs_file_path = ri->gr.abs_src_path;
	gp.rel_file_path = "";	
	
	render_folder(&ri->gr, &gp, req, con);
	
	strlist_free(gp.file_path_parts, 1);
	
	
	char* readme = git_get_file(&ri->gr, "master", "README.md");
	if(readme) {
		char* rm_safe = html_encode(readme, -1);
		cw("<table class=\"fileview\"><tr><th>", "README.md" ,"</th></tr><tr><td><pre lang=md>", rm_safe, "</pre></td></tr></table>");
		free(rm_safe);
		free(readme);
	}
	
	html_footer(con);
	
}





