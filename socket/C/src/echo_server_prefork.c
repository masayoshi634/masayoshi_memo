#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define STARTSERVERS 4

int main(int argc, char** argv)
{
    int sd;
    int acc_sd;
	int i, pid, status;
    struct sockaddr_in addr;

    socklen_t sin_size = sizeof(struct sockaddr_in);
    struct sockaddr_in from_addr;

    char buf[2048];

    memset(buf, 0, sizeof(buf));

    if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(50080);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }

    if(listen(sd, 10) < 0) {
        perror("listen");
        return -1;
    }

	for (i = 0; i < STARTSERVERS; i++){
		if ((pid = fork()) == 0) {
			int pid = getpid();
			printf("子プロセス(pid:%d)開始\n", pid);
			while (1){
				if((acc_sd = accept(sd, (struct sockaddr *)&from_addr, &sin_size)) < 0) {
					perror("accept");
					return -1;
				}
				if(recv(acc_sd, buf, sizeof(buf), 0) < 0) {
					perror("recv");
					return -1;
				}
				close(acc_sd);
			}
			printf("子プロセス(pid:%d)終了\n", pid);
		}
	}
	waitpid(pid, &status, 0);
    close(sd);
	return 0;
}
