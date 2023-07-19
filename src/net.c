
#include <stdio.h>

#include "net.h"


#define fatal(fmt, ...) do { \
	fprintf(stderr, "%s:%d " fmt, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
	exit(1); \
} while(0);




void add_epoll_watch(int epollfd, int fd, void* data, int events) {
	struct epoll_event ee = {0};
	
	ee.events = events;
	ee.data.ptr = data;

	int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ee);
	if(ret < 0) {
		printf("epoll error: %s\n", strerror(errno));
	}
}


server_t* server_init(int epollfd, int port) {
//	int epollfd = epoll_create(16);
	server_t* srv = calloc(1, sizeof(*srv));
	srv->epollfd = epollfd;
	
	srv->listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(srv->listen_socket, F_SETFL, O_NONBLOCK);
	
	int on = 1;
	setsockopt(srv->listen_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	
	struct sockaddr_in bindaddr, peeraddr;
	
	bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bindaddr.sin_family = AF_INET;
	bindaddr.sin_port = htons(port);
	
	if(bind(srv->listen_socket, (struct sockaddr*)&bindaddr, sizeof(bindaddr)) < 0) {
		fatal("Failed to bind socket");
	}
	
	listen(srv->listen_socket, SOMAXCONN);
	
	add_epoll_watch(epollfd, srv->listen_socket, srv->listen_socket, EPOLLIN);
	
	return srv;
}


void server_tick(server_t* srv, int wait) {

	struct epoll_event ee = {0};
//	printf("epoll waiting\n");
	
	int ret = epoll_wait(srv->epollfd, &ee, 1, wait); 
	
	if(ret == -1) {
		if(errno == EINTR) return;
		
		fatal("epoll error\n");
	}
	
	if(ret == 0) return;
	
	// new connections
	if(ee.data.fd == srv->listen_socket) {
		connection_t* con = calloc(1, sizeof(*con));
	
		int addrlen;
		memset(&con->peeraddr, 0, sizeof(con->peeraddr)); // shutup, valgrind
		con->peerfd = accept(srv->listen_socket, (struct sockaddr*)&con->peeraddr, &addrlen);
		fcntl(con->peerfd, F_SETFL, O_NONBLOCK);
	
		if(con->peerfd < 0) {
			printf("socket error: %s\n", strerror(errno));
	        exit(5);
	    }
		
		con->srv = srv;
		srv->on_accept(srv, con);
			
		add_epoll_watch(srv->epollfd, con->peerfd, con, EPOLLIN);
		
		VEC_PUSH(&srv->cons, con);
		
		return;
	}
	
	
	if(ee.events & EPOLLIN) { // data to read
//		printf("got data\n\n");
		
		connection_t* con = ee.data.ptr;
		
		int bread = 0;
		do {
			if(*con->buf_remain == 0) {
				con->buffer_full(con);
			}
			
			bread = recv(con->peerfd, *con->buf, *con->buf_remain, 0);
			
			if(bread > 0) {
				*con->buf_remain -= bread;
				con->new_data = 1;	
			}
			
	        if(errno == EAGAIN) {
				break;
			}
		} while(bread > 0);
		
		if(con->new_data) {
			printf("got new data\n");
			con->new_data = 0;
			if(con->got_data) con->got_data(con);
		}
	}
	
}


void connection_close(connection_t* con) {
	
	// flush the write buffer
	ssize_t wb, bytes_written = 0;
	if(con->write_buf && con->wb_len) {
		do {
			wb = send(con->peerfd, con->write_buf + bytes_written, con->wb_len - bytes_written, 0);
			bytes_written += wb;
		} while(wb > 0);
		
		free(con->write_buf);
	}
	
	// make sure we don't get any more events from epoll
	struct epoll_event ee = {0};
	epoll_ctl(con->srv->epollfd, EPOLL_CTL_DEL, con->peerfd, &ee);
				
	close(con->peerfd);
	
	VEC_RM_VAL(&con->srv->cons, con);
	
	free(con);
}

void connection_write(connection_t* con, char* s, ssize_t len) {
	if(len < 0) len = strlen(s);
	if(len == 0) return; 
	
	if(con->wb_len + len >= con->wb_alloc) {
		con->wb_alloc = nextPOT(con->wb_len + len + 1);
		con->write_buf = realloc(con->write_buf, con->wb_alloc);
	}
	
	memcpy(con->write_buf + con->wb_len, s, len);
	con->wb_len += len;
}




