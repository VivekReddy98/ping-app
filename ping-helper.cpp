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


// make a ping request
void ping(struct sockaddr_in *ping_addr)
{
	int msg_count=0, i, addr_len, flag=1, msg_received_count=0, ttl_val=64;

	struct icmpPKT pckt;
	struct sockaddr_in r_addr;
	struct timespec time_start, time_end, tfs, tfe;
	long double rtt_msec=0, total_msec=0;

  struct timeval tv_out;
	tv_out.tv_sec = RECV_TIMEOUT;
	tv_out.tv_usec = 0;

	clock_gettime(CLOCK_MONOTONIC, &tfs);


	// set socket options at ip to TTL and value to 64,
	// change to what you want by setting ttl_val
	if (setsockopt(socketFd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0)
		  error("\nSetting socket options to TTL failed! ");

	{
		printf("\nSocket set to TTL..\n");
	}

	// setting timeout of recv setting
	setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);

	// send icmp packet in an infinite loop
	while(!stop)
	{
		// flag is whether packet was sent or not
		flag=1;

		//filling packet
    memset(&pckt, '\0', sizeof(pckt));

		pckt.hdr.type = ICMP_ECHO;
		pckt.hdr.un.echo.id = getpid();

		// for ( i = 0; i < sizeof(pckt.msg)-1; i++)
		// 	pckt.msg[i] = '0';

		strcpy(pckt.msg, "ECHO");
		pckt.hdr.un.echo.sequence = msg_count++;
		pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));


		usleep(PING_SLEEP_RATE);

		//send packet
		clock_gettime(CLOCK_MONOTONIC, &time_start);
		if ( sendto(socketFd, &pckt, sizeof(pckt), 0, (struct sockaddr*)ping_addr, sizeof(*ping_addr)) <= 0)
		{
      flag=0;
			printf("\nPacket Transmission Failed!\n");
		}

		//receive packet
		addr_len = sizeof(r_addr);

		if ( recvfrom(socketFd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, (socklen_t*)&addr_len) <= 0 && msg_count>1)
		{
			printf("\nPacket receive failed!\n");
		}

		else
		{
			clock_gettime(CLOCK_MONOTONIC, &time_end);

			double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec))/1000000.0;
			rtt_msec = (time_end.tv_sec- time_start.tv_sec) * 1000.0 + timeElapsed;

			// if packet was not sent, don't receive
			if(flag)
			{
				if(!(pckt.hdr.type ==69 && pckt.hdr.code==0))
				{
					printf("Error..Packet received with ICMP type %d code %d\n", pckt.hdr.type, pckt.hdr.code);
				}
				else
				{
					printf("%d bytes from %s msg_seq=%d ttl=%d rtt = %Lf ms.\n", PACKETSIZE, inet_ntoa(ping_addr->sin_addr), msg_count, TTLVAL, rtt_msec);

					msg_received_count++;
				}
			}
		}
	}

  if (stop){
    clock_gettime(CLOCK_MONOTONIC, &tfe);
  	double timeElapsed = ((double)(tfe.tv_nsec - tfs.tv_nsec))/1000000.0;

  	total_msec = (tfe.tv_sec-tfs.tv_sec)*1000.0+timeElapsed;

  	printf("\n===%s ping statistics===\n", inet_ntoa(ping_addr->sin_addr));
  	printf("\n%d packets sent, %d packets received, %f percent packet loss. Total time: %Lf ms.\n\n",
                                          		msg_count, msg_received_count,
                                          		((msg_count - msg_received_count)/msg_count) * 100.0,
                                          		total_msec);
  }

}


void error(const char *msg) {
    perror(msg);
    exit(1);
}

void printtIP(struct sockaddr_in *sock_addr){
    char *IPbuffer;
    IPbuffer = inet_ntoa(sock_addr->sin_addr);
    printf("Host IP: %s\n", IPbuffer);
    printf("Port Allocated %d\n", sock_addr->sin_port);
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
