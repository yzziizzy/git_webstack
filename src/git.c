#include <stdlib.h>
#include <string.h>


#include "git.h"
#include "sys.h"





void free_git_repo(git_repo* gr) {
	free(gr->repo_name);
	free(gr->owner);
	free(gr->abs_base_path);
	free(gr->abs_src_path);
	free(gr->abs_meta_path);
	free(gr->abs_pulls_path);
	free(gr->abs_wiki_path);
	free(gr->abs_issues_path);
}


void free_git_issue(git_issue* gi) {
	free(gi->creator);
	free(gi->repo_name);
	free(gi->folder_name);
	free(gi->abs_path);
}


int git_repo_init_short(git_repo* gr, char* repos_path, char* target_repo) {
	char* slash = strchr(target_repo, '/');
	if(!slash) return 1;
	
	gr->owner = strndup(target_repo, slash -  target_repo);
	gr->repo_name = strdup(slash + 1);
	
	gr->abs_base_path = path_join(repos_path, "users", gr->owner, "repos", gr->repo_name);
	gr->abs_src_path = path_join(gr->abs_base_path, "src");
	gr->abs_meta_path = path_join(gr->abs_base_path, "meta");
	gr->abs_pulls_path = path_join(gr->abs_base_path, "pulls");
	gr->abs_wiki_path = path_join(gr->abs_base_path, "wiki");
	gr->abs_issues_path = path_join(gr->abs_base_path, "issues");
	
	return 0;
}



static int issue_search_cb(char* fullPath, char* fileName, void* data) {
	git_issue* gi = (git_issue*)data;
	
	char* s = sprintfdup("_%s#%d - ", gi->creator, gi->user_issue_num);
	if(strstr(fileName, s) == strchr(fileName, '_')) {
		gi->folder_name = strdup(fileName);
		free(s);
		return 1;
	}
	
	free(s);
	
	return 0;
} 


int git_issue_init_short(git_issue* gi, char* repos_path, char* target_issue) {
	memset(gi, 0, sizeof(*gi));

	char* slash1 = strchr(target_issue, '/');
	if(!slash1) return 1;
	
	char* slash2 = strchr(slash1 + 1, '/');
	if(!slash2) return 1;
	
	char* pound = strchr(slash2 + 1, '#');
	if(!pound) return 1;
	
	gi->repo_owner = strndup(target_issue, slash1 - target_issue);
	gi->repo_name = strndup(slash1 + 1, slash2 - slash1 - 1);
	gi->creator = strndup(slash2 + 1, pound - slash2 - 1);
	gi->user_issue_num = strtol(pound + 1, NULL, 10);
	
//	printf("owner: '%s'\n", gi->repo_owner);
//	printf("name: '%s'\n", gi->repo_name);
//	printf("creator: '%s'\n", gi->creator);
//	printf("num: '%d'\n", gi->user_issue_num);
	
	char* base = path_join(repos_path, "users", gi->repo_owner, "repos", gi->repo_name, "issues");
	char* open_path = path_join(base, "open");
	char* closed_path = NULL;

	// returns negative on error, nonzero if scanning was halted by the callback
	recurse_dirs(open_path, issue_search_cb, gi, 0,  FSU_EXCLUDE_FILES | FSU_INCLUDE_DIRS | FSU_NO_FOLLOW_SYMLINKS | FSU_EXCLUDE_HIDDEN);
	if(!gi->folder_name) {
	
		closed_path = path_join(base, "closed");
		
		// returns negative on error, nonzero if scanning was halted by the callback
		recurse_dirs(closed_path, issue_search_cb, gi, 0,  FSU_EXCLUDE_FILES | FSU_INCLUDE_DIRS | FSU_NO_FOLLOW_SYMLINKS | FSU_EXCLUDE_HIDDEN);
		if(!gi->folder_name) {
			// the issue doesn't exist
			return 1;
		}
		
		gi->open = 0;
	}
	else {
		gi->open = 1;
	}
	
	gi->abs_path = path_join(gi->open ? open_path : closed_path, gi->folder_name);
	
	free(open_path);
	free(closed_path);
	
	return 0;
}

int git_issue_init_from_dir(git_issue* gi, git_repo* gr, char* parent_dir, char* dir_name) {
	memset(gi, 0, sizeof(*gi));

	char* u = strchr(dir_name, '_');
	if(!u) return 1;
	
	char* n = strchr(u, '#');
	if(!n) return 1;
	
	char* e = strchr(n, ' ');
	if(!e) return 1;
	
	gi->repo_owner = strdup(gr->owner);
	gi->repo_name = strdup(gr->repo_name);
	gi->creator = strndup(u + 1, n - u - 1);
	gi->user_issue_num = strtol(n + 1, NULL, 10);
	gi->open = 0 == strcmp(parent_dir, "open");
	gi->parent_folder_name = strdup(parent_dir);
	
	gi->abs_path = path_join(gr->abs_issues_path, parent_dir, dir_name);
	
	return 0;
}






int git_issue_init(git_issue* gi, git_repo* gr, char* creator, char* creator_issue_num) {
	memset(gi, 0, sizeof(*gi));
	
	gi->repo_owner = strdup(gr->owner);
	gi->repo_name = strdup(gr->repo_name);
	gi->creator = strdup(creator);
	gi->user_issue_num = strtol(creator_issue_num, NULL, 10);
	
//	gi->parent_folder_name = strdup(parent_dir);
	
	//gi->open = 0 == strcmp(parent_dir, "open");
	
//	gi->abs_path = path_join(gr->abs_issues_path, parent_dir, dir_name);
	
	return 0;
}


char* git_count_commits(git_repo* gr, char* branch) {
	//	git rev-list --count <branch-name>
	return systemf("git --work-tree=%s --git-dir=%s/.git/ --no-pager rev-list --count %s",  gr->abs_src_path, gr->abs_src_path, branch);
}

long git_count_commits_int(git_repo* gr, char* branch) {
	
	char* s = git_count_commits(gr, branch);
	long l = strtol(s, NULL, 10);
	free(s);
	
	return l;
}


char* git_count_branches(git_repo* gr) {
	return sprintfdup("%ld", git_count_branches_int(gr));
}


long git_count_branches_int(git_repo* gr) {
	char* branches = git_get_local_branches(gr);
	
	long count = 0;
	for(char* s = branches; *s; s++) {
		if(*s == '\n') count++;
	}
	
	free(branches);
	
	return count;
}

char* git_get_local_branches(git_repo* gr) {
	return systemf("git --work-tree=%s --git-dir=%s/.git/ --no-pager branch -l",  gr->abs_src_path, gr->abs_src_path);
}



static int issue_count_cb(char* fullPath, char* fileName, void* data) {
	*((long*)data) += 1;
	return 0;
} 


long git_count_open_issues(git_repo* gr) {
	long count = 0;
	
	char* open_path = path_join(gr->abs_issues_path, "open");
	recurse_dirs(open_path, issue_count_cb, &count, 0,  FSU_EXCLUDE_FILES | FSU_INCLUDE_DIRS | FSU_NO_FOLLOW_SYMLINKS | FSU_EXCLUDE_HIDDEN);
	free(open_path);
	
	return count;
}


struct issuelist_data {
	git_issue_list* list;
	git_repo* gr;
	char* parent_dir;
};


static int issue_agg_cb(char* fullPath, char* fileName, void* data) {
	struct issuelist_data* d = (struct issuelist_data*)data;
	
	git_issue* gi = malloc(sizeof(*gi));
	
	if(!git_issue_init_from_dir(gi, d->gr, d->parent_dir, fileName)) {
		VEC_PUSH(&d->list->issues, gi);
	}
	else {
		free(gi);
	}
	
	
	return 0;
} 


git_issue_list* git_open_issues(git_repo* gr) {
	
	struct issuelist_data d = {
		.list = calloc(1, sizeof(git_issue_list)),
		.gr = gr,
		.parent_dir = "open",
	};
		
		
	char* open_path = path_join(gr->abs_issues_path, "open");
	recurse_dirs(open_path, issue_agg_cb, &d, 0,  FSU_EXCLUDE_FILES | FSU_INCLUDE_DIRS | FSU_NO_FOLLOW_SYMLINKS | FSU_EXCLUDE_HIDDEN);
	free(open_path);
	
	return d.list;
}


struct find_issue_dir_data {
	int found;
	char* key;
	char* base;
	char* folder;
	git_issue* gi;
};


static int issue_search3_cb(char* fullPath, char* fileName, void* data) {
	struct find_issue_dir_data* d = (struct find_issue_dir_data*)data;
printf(">> %s (%s)\n", fileName, d->key);
	if(strstr(fileName, d->key) == strchr(fileName, '_')) {
		d->gi->folder_name = strdup(fileName);
		d->gi->parent_folder_name = strdup(d->folder);
		d->found = 1;
		printf("found\n");
		return 1;
	}

	return 0;
}

static int issue_search2_cb(char* fullPath, char* fileName, void* data) {
	struct find_issue_dir_data* d = (struct find_issue_dir_data*)data;
	
	d->folder = fileName;
	char* path = path_join(d->base, fileName);
printf("\n> %s\n", path);
	recurse_dirs(path, issue_search3_cb, data, 1, FSU_EXCLUDE_FILES | FSU_INCLUDE_DIRS | FSU_NO_FOLLOW_SYMLINKS | FSU_EXCLUDE_HIDDEN);
	
	free(path);
	
	return d->found;
} 


void git_issue_find_dir(git_repo* gr, git_issue* gi) {
	
	struct find_issue_dir_data d = {
		.found = 0,
		.gi = gi,
		.folder = NULL,
		.base = gr->abs_issues_path,
		.key = sprintfdup("_%s#%d - ", gi->creator, gi->user_issue_num),
	};

	// returns negative on error, nonzero if scanning was halted by the callback
	recurse_dirs(gr->abs_issues_path, issue_search2_cb, &d, 1, FSU_EXCLUDE_FILES | FSU_INCLUDE_DIRS | FSU_NO_FOLLOW_SYMLINKS | FSU_EXCLUDE_HIDDEN);
	
	free(d.key);
}


// just the raw text
char* git_get_file(git_repo* gr, char* branch, char* path) {
	return systemf("git --work-tree=%s --git-dir=%s/.git/ --no-pager show %s:%s",  gr->abs_src_path, gr->abs_src_path, branch, path);
}



void git_repo_parse_request(git_repo* gr, strlist* uri_parts) {
	
	
	/*
	// extract username
//	if(0 && rm->enable_users) {
	if(sl->len >= 1) {
		gr->owner = strdup(uri_parts->entries[0]);
	}
	
	if(sl->len >= 2) {
		gr->repo_name = strdup(uri_parts->entries[1]);
	}
	
*/
	
	
	
	
}



