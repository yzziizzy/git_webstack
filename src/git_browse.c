#include <stdio.h>


#include "git_browse.h"
#include "cproc.h"
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>

#define cw(x) connection_write(con, x, -1)
#define cnw(x, l) connection_write(con, x, l)


static void html_header(connection_t* con) {
	cw("<html>\n<head><link rel=\"stylesheet\" type=\"text/css\" href=\"/_/main.css\" />\n<script type=\"text/javascript\" src=\"/_/main.js\"></script>\n</head>\n<body>\n");
}

static void html_footer(connection_t* con) {
	cw("\n</body>\n</html>\n");
}


char get_file_type(char* path) {
	struct stat st;
	int res = stat(path, &st);
	if(res) return 0;
	
	if(st.st_mode & S_IFDIR) return 'd';
	if(st.st_mode & S_IFREG) return 'f';
	if(st.st_mode & S_IFLNK) return 'l';
	
	return 0;
}

char* htmlencode(char* src, ssize_t len) {
	if(len < 0) len = strlen(src);
	
	int replacements = 0;
	for(int i = 0; src[i] && i < len; i++) {
		if(strchr("><&", src[i])) replacements++;
	}
	
	if(replacements == 0) {
		return strndup(src, len);
	}
	
	char* dst = malloc(len + replacements * 5 + 1);
	
	int d = 0;
	for(int i = 0; src[i] && i < len; i++) {
		switch(src[i]) {
			case '>':
				dst[d++] = '&';
				dst[d++] = 'g';
				dst[d++] = 't';
				dst[d++] = ';';
				break;
			case '<':
				dst[d++] = '&';
				dst[d++] = 'l';
				dst[d++] = 't';
				dst[d++] = ';';
				break;
			case '&':
				dst[d++] = '&';
				dst[d++] = 'a';
				dst[d++] = 'm';
				dst[d++] = 'p';
				dst[d++] = ';';
				break;
			default:
				dst[d++] = src[i];
		}
	}
	
	dst[d] = 0;
	
	return dst;
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


typedef struct {
	strlist* components;
	char* fullpath; // dirty atm
	char* repo_path;
	char* rel_path;

} path_info;



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

void do_folder(path_info* pi, scgi_request* req, connection_t* con) {


	char* cmd = sprintfdup("git --work-tree=%s --git-dir=%s/.git/ ls-tree --name-only HEAD ./%s/", pi->repo_path, pi->repo_path, pi->rel_path);
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
	for(int i = 0; i < pi->components->len; i++) {
		if(strlen(pi->components->entries[i]) == 0) continue;
		char* b = path_join(a, pi->components->entries[i]);
		cw("<a href=\"");
		cw(b);
		cw("\">");
		cw(pi->components->entries[i]);
		cw("</a>");
		
		free(a);
		a = b;
		
		if(i < pi->components->len - 1) {
			cw("/");
		}
	}
	cw("</th></tr>");
	
	free(a);
	
	for(char** s = files; *s; s++) {
		char* cmd = sprintfdup(cmd_fmt, pi->repo_path, pi->repo_path, *s);
		char* res = sysstring(cmd);
		printf("cmd: %s\n", cmd);
		free(cmd);
		
		char** data = str_split(res, "\r\n");
		cw("<tr>");
		
		cw("<td><a href=\"/");
		cw(pi->components->entries[1]); cw("/"); cw(*s);
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

void do_file(path_info* pi, scgi_request* req, connection_t* con) {


	char* cmd = sprintfdup("git --work-tree=%s --git-dir=%s/.git/  --no-pager blame -lc %s", pi->repo_path, pi->repo_path, pi->fullpath);
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
	for(int i = 0; i < pi->components->len; i++) {
		if(strlen(pi->components->entries[i]) == 0) continue;
		char* b = path_join(a, pi->components->entries[i]);
		cw("<a href=\"");
		cw(b);
		cw("\">");
		cw(pi->components->entries[i]);
		cw("</a>");
		
		free(a);
		a = b;
		
		if(i < pi->components->len - 1) {
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
		char* textsafe = htmlencode(text, -1);
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
	
	
	
	
	char** uri_parts = str_split(uri, "/");
	char** part = uri_parts;
	
	if(uri_parts[0] == NULL || (strlen(uri_parts[0]) == 0 && uri_parts[1] == NULL)) {
		do_project_index(rm, req, con);
		
		free_strpp(uri_parts);
		return;
	}

	strlist* final_path = strlist_new();
	
	for(; *part; part++) {
		char* a = join_str_list(final_path->entries, "/");
		char* b = path_join(rm->path, a);
		char* c = path_join(b, *part);
		
		printf("\n");
		
		char type = get_file_type(c);
		if(type == 'd') {
			printf("is dir\n");
			strlist_push(final_path, *part);
		}
		else if(type == 'f') {
			printf("is file\n");
			strlist_push(final_path, *part);
			break;
		}
		
		printf("part '%s'\n", *part);
		printf("a: '%s'\n", a);
		printf("b: '%s'\n", b);
		printf("c: '%s'\n", c);
			
		free(a);
		free(b);
		free(c);
	}
	
	
	char* relpath = join_str_list(final_path->entries, "/");
	char* path = path_join(rm->path, relpath);
	
	
	
	printf("orig path: '%s'\n", path);
	
	if(strstr(uri, "..")) {
		printf("'..' found in request uri\n");
		cw("Status: 404\r\n\r\n");
		return;
	}
	
	
	char type = get_file_type(path);
	
	
	char* base = strlen(uri_parts[0]) == 0 ? uri_parts[1] : uri_parts[0];
	
	path_info pi = {
		.components = final_path,
		.fullpath = path,
		.repo_path = path_join(rm->path, base),
		.rel_path = strchrnul(relpath + 1, '/'),
	};
	
	if(type == 'd') {
		do_folder(&pi, req, con); 
		return;
	}
	else if(type == 'f') {
		do_file(&pi, req, con);
		return;
	}
	else {
		cw("Status: 404\r\n\r\n");
		return;
	}
	
	free(relpath);
	free(pi.repo_path);
	free_strpp(uri_parts);
	strlist_free(final_path, 1);
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
	
			
}


