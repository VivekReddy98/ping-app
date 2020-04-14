#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <string.h>

#include "ping.h"

extern int socketFd;
extern int TTLVAL;
extern double RECV_TIMEOUT;

extern volatile sig_atomic_t stop;

unsigned short checksum(void *b, int len)
{
    unsigned short *buf = (unsigned short *)b;
  	unsigned int sum=0;
  	unsigned short result;

  	for ( sum = 0; len > 1; len -= 2 )
  		sum += *buf++;
  	if ( len == 1 )
  		sum += *(unsigned char*)buf;
  	sum = (sum >> 16) + (sum & 0xFFFF);
  	sum += (sum >> 16);
  	result = ~sum;
  	return result;
}


// Interrupt handler
void interruptHandler(int signum)
{
	stop=1;
}

int checkforError(struct replyPKT *rlpy_pckt, struct sockaddr_in *ping_addr){
    int retval = 1;
    //||  rlpy_pckt->hdr.typ == 3
    if (rlpy_pckt->hdr.type == 3){
       if (rlpy_pckt->hdr.code == 0)
          printf("\n0 bytes from %s : Network unreachable", inet_ntoa(ping_addr->sin_addr));
       else if (rlpy_pckt->hdr.code == 1)
          printf("\n0 bytes from %s : Host unreachable", inet_ntoa(ping_addr->sin_addr));
       else if (rlpy_pckt->hdr.code == 2)
          printf("\n0 bytes from %s : Protocol unreachable", inet_ntoa(ping_addr->sin_addr));
       else if (rlpy_pckt->hdr.code == 3)
          printf("\n0 bytes from %s : Port unreachable", inet_ntoa(ping_addr->sin_addr));
    }
    else if (rlpy_pckt->hdr.type == 11){
       printf("\n 0 bytes from %s : TTL Exceeded", inet_ntoa(ping_addr->sin_addr));
    }
    else {
      retval = 0;
    }

    return retval;
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void printtIP(struct sockaddr_in *sock_addr){
    char *IPbuffer;
    IPbuffer = inet_ntoa(sock_addr->sin_addr);
    printf("Host IP: %s\n", IPbuffer);
}


struct sockaddr_in resolveDNS(char *host_name){
  struct hostent* host;
  struct sockaddr_in serv_addr;

  if ((host = gethostbyname((const char *)host_name)) == NULL)
  {
      error("Error in Resolving ipaddr or the hostname provided !");
  }

  memcpy(&serv_addr.sin_addr, host->h_addr_list[0], host->h_length);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PING_PORT);

  return serv_addr;
}
