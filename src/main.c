#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sti/sti.h"

#include "net.h"
#include "scgi.h"








int main(int argc, char* argv[]) {

	
	server_t* srv = scgi_create(4999);
	

	
	while(1) {
		
		server_tick(srv, 100);
		
//		sleep(1);
	}


	close(srv->epollfd);

	return 0;
}









