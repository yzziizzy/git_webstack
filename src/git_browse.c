#include <stdio.h>


#include "git_browse.h"
#include "cproc.h"
#include <ctype.h>

#define cw(x) connection_write(con, x, -1)
#define cnw(x, l) connection_write(con, x, l)


static void html_header(connection_t* con) {
	cw("<html>\n<head><link type=\"text/css\" href=\"/_/main.css\" />\n<script src=\"/_/main.js\"></script>\n</head>\n<body>\n");
}

static void html_footer(connection_t* con) {
	cw("\n</body>\n</html>\n");
}


static char* sysstring(char* cmdline) {
	struct child_process_info* cpi = exec_cmdline_pipe(cmdline);

	
	while(1) {	
		read_cpi(cpi);	
		
		int status;
		// returns 0 if nothing happened, -1 on error, childpid if it exited
		int pid = waitpid(cpi->pid, &status, WNOHANG);
		if(pid != 0) {
			
			read_cpi(cpi);
			
			cpi->output_buffer[cpi->buf_len] = 0;
			cpi->exit_status = WEXITSTATUS(status);
			
			break;
		}
		
		usleep(100);
	}
	
	char* out = cpi->output_buffer;

	free_cpi(cpi, 0);
	
	return out;
}



void do_folder(char* path, scgi_request* req, connection_t* con) {


	char* cmd = sprintfdup("git ls-tree --name-only HEAD %s", path);
	char* str = sysstring(cmd);
	free(cmd);
	
//	printf("'%s'\n", str);
	char** files = str_split(str, "\r\n");
	
	char* cmd_fmt = "git --no-pager log -1 --pretty=format:%%H%%n%%cn%%n%%ce%%n%%ci%%n%%cr%%n%%s%%n %s";
	
	
	
	char* resp = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
		
	cw(resp);
	html_header(con);
	
	
	cw("<table>");
	cw("<tr><th colspan=4>");
	cw(path);
	cw("</th></tr>");
	
	for(char** s = files; *s; s++) {
		char* cmd = sprintfdup(cmd_fmt, *s);
		char* res = sysstring(cmd);
		free(cmd);
		
		char** data = str_split(res, "\r\n");
		cw("<tr>");
		
		cw("<td><a href=\"");
		cw(*s);
		cw("\">");
		cw(*s);
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

void do_file(char* path, scgi_request* req, connection_t* con) {


	char* cmd = sprintfdup("git --no-pager blame -lc ./%s", path);
	char* str = sysstring(cmd);
	free(cmd);
	
//	printf("'%s'\n", str);
	char** files = str_split(str, "\r\n");

	char* resp = "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n";
	
	cw(resp);
	html_header(con);
	
	cw("<table>");
	
	cw("<tr><th colspan=4>");
	cw(path);
	cw("</th></tr>");
	
	
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
		
		cw("<td>");
		cnw(line, line_end - line);
		cw("</td>\n");
		
		cw("<td><pre>");
		cw(text);
		cw("</pre></td>\n");
		
		
		cw("</tr>\n");
		
	}
	cw("</table>");
	
	
	html_footer(con);

	free(str);
	free_strpp(files);

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
	
	if(0 == strncmp(uri, "/_/", 3)) {
		char* resp = "Status: 404\r\n\r\n";
		cw(resp);
		printf("/_/ denied\n");
		return;
	}
	
	
	
	char* path = path_join(rm->path, uri);
	
	if(0 == strcmp(path, "/")) {
		do_folder(".", req, con); 
		return;
	}
	
	do_file(path, req, con);
	

/*
	
	for(char** s = files; *s; s++) {
		
		for(int i = 0; (*s)[i]; i++) {
			if(isalnum((*s)[i])) printf("%c", (*s)[i]);
			else printf(" %d ", (int)(*s)[i]);
		}
		printf("\n");
		cw(*s, strlen(*s), 0);
		cw("\r\n", strlen("\r\n"), 0);
			
	}
	printf("\n");
	*/
	
	cw("\r\n");
			
}


