#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "sti/sti.h"

#include "net.h"
#include "scgi.h"
#include "git_browse.h"








int main(int argc, char* argv[]) {
	struct sigaction a;
	memset(&a, 0, sizeof(a));
	a.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &a, NULL);

	repo_meta* rm = calloc(1, sizeof(*rm));
	rm->path = "./"; // because the executable is running from the project's repo
	rm->static_asset_path = "./webstatic";
	
	scgi_server* srv = scgi_create(4999, rm, git_browse_handler);
	srv->handler = git_browse_handler;
	srv->user_data = rm;

	
	while(1) {
		
		server_tick(srv->srv, 100);
		
//		sleep(1);
	}


	close(srv->srv->epollfd);

	return 0;
}









