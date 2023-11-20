#include <stdio.h>



#include "../git_browse.h"
#include "../cproc.h"
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>





void do_file(request_info* ri, scgi_request* req, connection_t* con) {
	printf("rendering file\n");

	char* cmd = sprintfdup("git --work-tree=%s --git-dir=%s/  --no-pager blame -lc master -- %s", ri->abs_src_path, ri->abs_src_path, ri->rel_file_path);
	char* str = sysstring(cmd);
	free(cmd);
	
//	printf("'%s'\n", str);
	char** files = str_split(str, "\r\n");

	char* resp = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
	
	cw(resp);
	html_header(con);
	html_repo_header(con, &ri->gr);
	
	cw("<table class=\"codelisting lang-c\">");
	
	char* a = strdup("");
	
	cw("<tr><th colspan=4>");
	
	for(int i = 0; i < ri->file_path_parts->len; i++) {
		if(strlen(ri->file_path_parts->entries[i]) == 0) continue;
		char* b = path_join(a, ri->file_path_parts->entries[i]);
		cw("<a href=\"/");
		cw(ri->username); cw("/"); cw(ri->project); cw("/src/"); cw(ri->branch);
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



