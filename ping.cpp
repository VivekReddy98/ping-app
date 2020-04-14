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
#include "ping.h"


// Socket Fd
int socketFd;
int TTLVAL;
double RECV_TIMEOUT;
int pingflag;


int main(int argc, char *argv[]){

    if (argc != 2){
       error("Number of Arguments must be specified are three: \
                  ./ping {ipaddrv4 | hostname | ipaddrv6}");
    }
    struct sockaddr_in sock_addr = resolveDNS(argv[1]);

    printtIP(&sock_addr);

    socketFd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(socketFd<0)
        error("Socket Error!!\n");

    // Setting Global Variables
    TTLVAL = 300;
    RECV_TIMEOUT = 1.;

    pingflag = 1;

    ping(&sock_addr);

    signal(SIGINT, interruptHandler);
    
    return 1;

}
