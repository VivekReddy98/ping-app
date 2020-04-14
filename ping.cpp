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

    // Get the Hostname and IpAdrdr
    struct sockaddr_in sock_addr = resolveDNS(argv[1]);

    // Print The IP Address
    printtIP(&sock_addr);

    // Set a Raw Socket
    socketFd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(socketFd<0)
        error("Socket Error!!\n");

    // Set Atomic Variable to handle User interrupt
    stop = 0;
    signal(SIGINT, interruptHandler);

    // Ping Function
    ping(&sock_addr, TTLVAL);
    return 1;
}


// make a ping request
void ping(struct sockaddr_in *ping_addr, int ttl_val)
{
	int msg_count=0, i, addr_len, flag=1, msg_received_count=0;

  // Variable & Flags for Recieveing Packet
  int activity, rcvFlag, errChk;

  //set of socket descriptors
  fd_set readfds;

  // Send Packet Struc
	struct icmpPKT pckt;

  // Rcv Paket Struct
  struct replyPKT rlpy_pckt;

  // Other Required Structs and time related variables
  struct sockaddr_in r_addr;
  struct timespec time_start, time_end, tfs, tfe;
	long double rtt_msec=0, total_msec=0;

  struct timeval tv_out;
	tv_out.tv_sec = 1.;
	tv_out.tv_usec = 0;

  // Clock the Start Time
	clock_gettime(CLOCK_MONOTONIC, &tfs);


	// set socket options at ip to TTL and value to the parameter specified in ttl_val,
	if (setsockopt(socketFd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0)
		  error("Setting socket options to TTL failed! \n");

	// setting timeout of recv setting
	setsockopt(socketFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);

	// send icmp packet in an infinite loop
	while(!stop)
	{
		// flag is whether packet was sent or not
		flag=1;

		//filling packet with Required Information
    memset(&pckt, '\0', sizeof(pckt));

		pckt.hdr.type = ICMP_ECHO;
		pckt.hdr.un.echo.id = getpid();
		strcpy(pckt.msg, "ECHO");
		pckt.hdr.un.echo.sequence = msg_count++;
		pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

    // Sleep With Waittime Defaults to one Second
		usleep(PING_SLEEP_RATE);

		//send packet
		clock_gettime(CLOCK_MONOTONIC, &time_start);
		if (sendto(socketFd, &pckt, sizeof(pckt), 0, (struct sockaddr*)ping_addr, sizeof(*ping_addr)) <= 0)
		{
      flag=0;
			printf("\nPacket Transmission Failed!\n");
		}

    // Create FD_Sets to recieve the Packet;
    FD_ZERO(&readfds);
    FD_SET(socketFd, &readfds);

    // Monitor Activity on the Fd
    activity = select(socketFd + 1 , &readfds , NULL , NULL , &tv_out);
    if (activity == -1)
       error("selcect () error");
    else{
       addr_len = sizeof(r_addr);
       rcvFlag = recvfrom(socketFd, &rlpy_pckt, sizeof(rlpy_pckt), 0, (struct sockaddr*)&r_addr, (socklen_t*)&addr_len);

       if (rcvFlag == -1){
         if (errno == EAGAIN)
          error("Wait Time Exceeded");
       }
       else
   		 {
     			clock_gettime(CLOCK_MONOTONIC, &time_end);
     			// if packet was not sent, don't receivec
     			if(flag)
     			{
            errChk = checkforError(&rlpy_pckt, ping_addr);

     				if(!errChk)
     				{
              // Calculate time required
              double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec))/1000000.0;
         			rtt_msec = (time_end.tv_sec- time_start.tv_sec) * 1000.0 + timeElapsed;
              printf("%d bytes from %s msg_seq=%d ttl=%d rtt = %Lf ms.\n", PACKETSIZE, inet_ntoa(ping_addr->sin_addr), msg_count, ttl_val, rtt_msec);
     					msg_received_count++;
     				}
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
