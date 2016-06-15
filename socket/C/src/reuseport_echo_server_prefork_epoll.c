#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <string.h>
#include "unistd.h"

#define STARTSERVERS 10
#define FD_SIZE 10000
#define MAX_EVENTS 100000

int main(int argc, char** argv)
{
    int sd;
    int acc_sd;
	int i, pid, status;
    struct sockaddr_in addr;
	char *res = "HTTP/1.0 200 OK\nContent-Type: text/html\n\nOK\n";

    socklen_t sin_size = sizeof(struct sockaddr_in);
    struct sockaddr_in from_addr;

    char buf[2048];

    memset(buf, 0, sizeof(buf));


	for (i = 0; i < STARTSERVERS; i++){
		if ((pid = fork()) == 0) {

			if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				perror("socket");
				return -1;
			}

			addr.sin_family = AF_INET;
			addr.sin_port = htons(50080);
			addr.sin_addr.s_addr = INADDR_ANY;

			const int on = 1;
			setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
			setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));

			if(bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
				perror("bind");
				return -1;
			}

			if(listen(sd, 10) < 0) {
				perror("listen");
				return -1;
			}
			int epfd;
			struct epoll_event ev, events[MAX_EVENTS];
			epfd = epoll_create(FD_SIZE);
			int pid = getpid();

			printf("子プロセス(pid:%d)開始\n", pid);

			memset(&ev, 0, sizeof ev);
			ev.events = EPOLLIN;
			ev.data.fd = sd;
			printf("epoll %d\n", pid);
			epoll_ctl(epfd, EPOLL_CTL_ADD, sd, &ev);

			while (1){
				int nfd = epoll_wait(epfd, events, MAX_EVENTS, -1);
				if (nfd <= 0) {
				  perror("epoll_wait");
				  return 1;
				}

				for (i = 0; i < nfd; i++) {
					if(events[i].data.fd == sd){
						if((acc_sd = accept(sd, (struct sockaddr *)&from_addr, &sin_size)) < 0) {
							perror("accept");
							return -1;
						}
						memset(&ev, 0, sizeof ev);
						ev.events = EPOLLIN;
						ev.data.fd = acc_sd;
						//printf("epoll %d\n", pid);
						epoll_ctl(epfd, EPOLL_CTL_ADD, acc_sd, &ev);
					}else{
						if(recv(events[i].data.fd, buf, sizeof(buf), 0) < 0) {
							perror("recv");
							return -1;
						}
						//printf("%s\n", buf);
						usleep(10000);
						send(events[i].data.fd, res, strlen(res), 0);
						close(events[i].data.fd);
					}
				}
			}
			printf("子プロセス(pid:%d)終了\n", pid);
		}
	}
	waitpid(pid, &status, 0);
    close(sd);
	return 0;
}
