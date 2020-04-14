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

#include <iostream>
#include <string.h>

#define PING_PORT 8989  // PingPort Specified
#define PACKETSIZE	64
#define PING_SLEEP_RATE 1000000

extern int socketFd;
extern int TTLVAL;
extern double RECV_TIMEOUT;
extern volatile sig_atomic_t stop;

struct icmpPKT
{
	struct icmphdr hdr;
	char msg[PACKETSIZE-sizeof(struct icmphdr)];
};

struct replyPKT
{
		struct iphdr ipHdr;
		struct icmphdr hdr;
		char msg[PACKETSIZE-sizeof(struct icmphdr)];
};


void error(const char *msg);
void printtIP(struct sockaddr_in *sock_addr);
struct sockaddr_in resolveDNS(char *host_name);
unsigned short checksum(void *b, int len);
void interruptHandler(int dummy);
void ping(struct sockaddr_in *ping_addr, int ttl_val, double recv_timeout);
int checkforError(struct replyPKT *rplyPKT, struct sockaddr_in *ping_addr);
