#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "git.h"
#include "sys.h"
#include "sti/sti.h"



char* strip_nonfs_bullshit(char* issue_title) {
	char* out = strdup(issue_title);
	char* r, *w;
	int last_was_space = 0;
	
	for(r = out, w = out; *r; r++) {
		if(isalnum(*r) || strchr("_+=-,.%", *r)) {
			*w = *r;
			w++;
			last_was_space = 0;
		}
		else if(isspace(*r)) {
			if(!last_was_space) {
				*w = ' ';
				w++;
			}
			last_was_space = 1;
		}
	}
	
	*w = 0;
	
	return out;
}


int create_issue(git_repo* gr, char* issue_username, char* issue_file_path) {
	int ret = 1;
	FILE* f = 0;
	
	memtricks_set_shitty_arena();

	char* open_path = path_join(gr->abs_issues_path, "open");
	
	if(!is_dir(open_path)) {
		fprintf(stderr, "Error: '%s' is not a directory.\n", open_path);
		goto FAIL;
	}
	
	char* file_contents = get_file(issue_file_path);
	char* issue_title = extract_line(file_contents, "Title");
	
	char* issue_title_safe = strip_nonfs_bullshit(issue_title);
	if(0 == strlen(issue_title_safe)) {
		fprintf(stderr, "Error: Sanitized issue title is empty.\n");
		goto FAIL;
	}
	
	
	time_t unixt = time(NULL);
	struct tm* tm = gmtime(&unixt);
	
	
	int user_issue_num = 1;
	char* abs_issue_path;
	char* issuedir;
	
	for(int limit = 0; limit < 100; limit++) {
		
		issuedir = sprintfdup("%d-%.2d-%.2d_%s#%d - %.32s", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, issue_username, user_issue_num, issue_title_safe);
		abs_issue_path = path_join(open_path, issuedir);
	
		if(mkdir(abs_issue_path, 0777)) {
			if(errno == EEXIST) {
				user_issue_num++;
				continue;
			}
			
			fprintf(stderr, "Error: Issue directory could not be created.\n");
			goto FAIL;
		}
		
		break;
	}
	
	char* abs_issue_file = sprintfdup("%s/%ld_%s", abs_issue_path, unixt, issue_username);
	f = fopen(abs_issue_file, "wb");
	if(!f) {
		printf("%s\n", strerror(errno));
		fprintf(stderr, "Error: could not create issue comment file '%s'\n", abs_issue_file);
		goto FAIL;
	}
	
	char* msg = strstr(file_contents, "\n\n");
	if(!msg) {
		fprintf(stderr, "Error: No message in issue file '%s'.\n", issue_file_path);
		goto FAIL;
	}
	msg += 2; // skip the linebreaks
	
	char* header = sprintfdup("Date: %d-%.2d-%.2d %.2d:%.2d\nTimestamp: %ld\nAuthor: %s\nRepo User: %s\nRepo Name: %s\nTitle: %s\n\n%s", 
		tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, unixt, issue_username, gr->owner, gr->repo_name, issue_title, msg);
	
	if(file_write_string(f, header, -1)) {
		fprintf(stderr, "Error: could not write to issue comment file");
		goto FAIL;
	}
	
//	if(write_whole_file()
	
	ret = 0;
	
FAIL:
	if(f) fclose(f);

	memtricks_shitty_arena_exit();
	
	return ret;
	
}
	




















