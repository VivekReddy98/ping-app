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

#define PING_PORT 8989
#define PACKETSIZE	64
#define PING_SLEEP_RATE 1000000

extern int socketFd;
extern int TTLVAL;
extern double RECV_TIMEOUT;
extern volatile sig_atomic_t stop;

// Send Packet Structure
struct icmpPKT
{
		struct icmphdr hdr;
		char msg[PACKETSIZE-sizeof(struct icmphdr)];
};

// Recieve Packet Structure
struct replyPKT
{
		struct iphdr ipHdr;
		struct icmphdr hdr;
		char msg[PACKETSIZE-sizeof(struct icmphdr)];
};

// Print Error with errno flag
void error(const char *msg);

// Print IP address
void printtIP(struct sockaddr_in *sock_addr);

// resolveDNS to get the Ip Address
struct sockaddr_in resolveDNS(char *host_name);

// Function to compute Checksum
unsigned short checksum(void *b, int len);

// Handle Cntrl+c interrupt
void interruptHandler(int dummy);

// Ping function
void ping(struct sockaddr_in *ping_addr, int ttl_val);

// Check and Report ping Type 3 and Type 11 errors
int checkforError(struct replyPKT *rplyPKT, struct sockaddr_in *ping_addr);
