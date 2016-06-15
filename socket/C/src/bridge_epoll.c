#include <stdio.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "rawsocketutil.h"
#include "printheader.h"

#define BUFFER_SIZE 2048
#define FD_SIZE 100
#define MAX_EVENTS 10

int analyze_packet(char *data){
	struct ether_header *eh;
	struct ip_header *ip;
	struct ether_arp *arp;
	int ip_option_len;
	u_char *ip_option;
	int epfd;

	eh = (struct ether_header *)data;
	data += sizeof(struct ether_header);
	PrintEtherHeader(eh, stdout);
	switch(ntohs(eh->ether_type)){
		case 0x800:
			ip = (struct ip_header *)data;
			PrintIpHeader(ip, ip_option, ip_option_len, stdout);
			break;
		case 0x806:
			arp = (struct ether_arp *)data;
			PrintArpHeader(arp, stdout);
			break;
		default:
			fprintf(stdout, "Unknown protocol\n");
	}
	fprintf(stdout, "=============================================\n");
}

int main(int argc, char *argv[], char *envp[]){
	int sock1, sock2;
	u_char buf[2048];
	u_char *data;
	u_char *ip_option;
	struct ether_header *eh;
	struct ip_header *ip;
	struct ether_arp *arp;
	int ip_option_len;
	int epfd;
	struct epoll_event ev, events[MAX_EVENTS];

	if(argc == 2){
		fprintf(stderr, "sudo sniff INTERFACE_NAME(ex,'eth0')");
	}

	sock1 = init_raw_socket(argv[1], 1, 0);
	if(sock1 == -1){
		printf("init_raw_socket error");
		return(-1);
	}
	sock2 = init_raw_socket(argv[2], 1, 0);
	if(sock2 == -1){
		printf("init_raw_socket error");
		return(-1);
	}

	epfd = epoll_create(FD_SIZE);

	memset(&ev, 0, sizeof ev);
	ev.events = EPOLLIN;
	ev.data.fd = sock1;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sock1, &ev);

	memset(&ev, 0, sizeof ev);
	ev.events = EPOLLIN;
	ev.data.fd = sock2;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sock2, &ev);

	while(1){
        int i;
        int nfd = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (nfd <= 0) {
		  perror("epoll_wait");
		  return 1;
		}

        for (i = 0; i < nfd; i++) {
			if(events[i].data.fd == sock1){
				int read_size = read(sock1, buf, sizeof(buf));
				analyze_packet(buf);
				write(sock2, buf, read_size);
			}else if(events[i].data.fd == sock2){
				int read_size = read(sock2, buf, sizeof(buf));
				analyze_packet(buf);
				write(sock1, buf, read_size);
			}
		}
	}
}
