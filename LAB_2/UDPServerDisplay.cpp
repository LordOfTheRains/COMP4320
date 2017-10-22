/*
** Original  listener.c by Beej -- a datagram sockets "server" demo
** Modified by Saad Biaz 08/29/2014
** UDPServerDisplay
** Modification 1 (_M1): Can bind to a port # provided on the command line
** Modification 2 (_M2): Server must loop indefinitely
** Modification 3 (_M3): Display Datagram as individual bytes in hexadecimal
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "ServerUDP.h"

// _M1 Not needed anymore #define MYPORT "10010"  // the port users will be connecting to

#define MAXBUFLEN 100

void displayBuffer(char *Buffer, int length); // _M3

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// _M1 , now we need arguments int main(void)
int main(int argc, char *argv[]) // _M1
{
	int port;
	/* _M1 Begin */
	if (argc != 2) {
		fprintf(stderr,"usage: UDPServerDisplay Port# \n");
		exit(1);
	}
	/* _M1 End*/

  port = atoi(argv[1]);
	ServerUDP server = ServerUDP(port);
	int err = server.configure();
	if (err){
		puts("error configuring udp server \n");
		return 0;
	}else{
		puts("server started... \n");
		server.run();
	}
}
