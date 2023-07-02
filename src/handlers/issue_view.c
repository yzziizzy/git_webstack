#include <stdio.h>


#include "../git_browse.h"
#include "../cproc.h"
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>





static int issue_cat_cb(char* fullPath, char* fileName, void* data) {
	connection_t* con = (connection_t*)data;

	cw("<pre class=\"issue\">");
	cw(get_file(fullPath));
	cw("</pre><hr />");
	
	return 0;
}



void do_issue(request_info* ri, git_issue* gi, scgi_request* req, connection_t* con) {
	
//	memtricks_set_shitty_arena();
	
	cw("Status: 200 OK\r\nContent-Type: text/html\r\n\r\n");
	
	html_header(con);
	html_repo_header(con, &ri->gr);
	
	cw("<h1>Issue</h1>");
		
	// TEMP hacky bullshit
	git_issue_find_dir(&ri->gr, gi);
	gi->abs_path = path_join(ri->gr.abs_issues_path, gi->parent_folder_name, gi->folder_name);
		
	
	recurse_dirs(gi->abs_path, issue_cat_cb, con, 1, FSU_NO_FOLLOW_SYMLINKS | FSU_EXCLUDE_HIDDEN);
	

	
	html_footer(con);
	
//	memtricks_shitty_arena_exit();
}










