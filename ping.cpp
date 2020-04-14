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

// Defualt Values
int TTLVAL = 255;
double RECV_TIMEOUT = 1.; // Seconds

volatile sig_atomic_t stop;

int main(int argc, char *argv[]){

    if (argc < 2){
       error("Number of Arguments must be specified are three: \
                  ./ping {ipaddrv4 | hostname} [-t TTL (TTL < 255)]");
    }
    else if (argc >= 4){
      if (strcmp(argv[2], "-t") == 0)
          TTLVAL = atoi(argv[3]);
      else{
         printf("%s Argument Ignored\n", argv[3]);
      }
    }

    TTLVAL = TTLVAL & 0x00ff; // Limiting the TTL between 0-255

    struct sockaddr_in sock_addr = resolveDNS(argv[1]);

    printtIP(&sock_addr);

    socketFd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(socketFd<0)
        error("Socket Error!!\n");

    stop = 0;
    signal(SIGINT, interruptHandler);

    ping(&sock_addr, (int )TTLVAL, RECV_TIMEOUT);
    return 1;
}
