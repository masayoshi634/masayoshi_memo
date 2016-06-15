#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/ioctl.h>
#include	<arpa/inet.h>
#include	<sys/socket.h>
#include	<linux/if.h>
#include	<net/ethernet.h>
#include	<netpacket/packet.h>
#include	<netinet/if_ether.h>
#include    "rawsocketutil.h"

int init_raw_socket(char *device,int promiscFlag,int ipOnly){
	struct ifreq ifreq;
	struct sockaddr_ll sa;
	int soc;

	if(ipOnly){
		if((soc=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_IP)))<0){
			printf("socket");
			return(-1);
		}
	}
	else{
		if((soc=socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL)))<0){
			printf("socket");
			return(-1);
		}
	}

	memset(&ifreq,0,sizeof(struct ifreq));
	strncpy(ifreq.ifr_name,device,sizeof(ifreq.ifr_name)-1);
	if(ioctl(soc,SIOCGIFINDEX,&ifreq)<0){
		printf("ioctl");
		close(soc);
		return(-1);
	}
	sa.sll_family=PF_PACKET;
	if(ipOnly){
		sa.sll_protocol=htons(ETH_P_IP);
	}
	else{
		sa.sll_protocol=htons(ETH_P_ALL);
	}
	sa.sll_ifindex=ifreq.ifr_ifindex;
	if(bind(soc,(struct sockaddr *)&sa,sizeof(sa))<0){
		printf("bind");
		close(soc);
		return(-1);
	}

	if(promiscFlag){
		if(ioctl(soc,SIOCGIFFLAGS,&ifreq)<0){
			printf("ioctl");
			close(soc);
			return(-1);
		}
		ifreq.ifr_flags=ifreq.ifr_flags|IFF_PROMISC;
		if(ioctl(soc,SIOCSIFFLAGS,&ifreq)<0){
			printf("ioctl");
			close(soc);
			return(-1);
		}
	}
	return(soc);
}
