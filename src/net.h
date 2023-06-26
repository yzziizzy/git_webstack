#ifndef __GWS__net_h__
#define __GWS__net_h__


#include <errno.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "sti/sti.h"

struct server;



typedef struct connection {
	struct server* srv;
	int peerfd;
	struct sockaddr_storage peeraddr;
	
	char** buf;
	size_t* buf_remain;
	
	int new_data;
	
	void (*buffer_full)(struct connection*);
	void (*got_data)(struct connection*);
	
	void* user_data;
	
	char* write_buf;
	size_t wb_len, wb_alloc;	
	
} connection_t;


typedef struct server {
	int epollfd;
	
	int listen_socket;
	
	VEC(connection_t*) cons;
	
	void (*on_accept)(struct server*, connection_t*);
	
	void* user_data;
	
} server_t;


server_t* server_init(int epollfd, int port);
void server_tick(server_t* srv, int wait);
void connection_close(connection_t* con);
void connection_write(connection_t* con, char* s, ssize_t len);

#endif // __GWS__net_h__
