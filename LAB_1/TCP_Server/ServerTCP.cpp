/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <iostream>

#define PORT "10017"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXDATASIZE 256 

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct message
{
	unsigned char tml;
	unsigned short requestID;
	unsigned long result; 
} __attribute__((__packed__)); 

typedef struct message message_t; 

struct received 
{
	unsigned char tml;
	unsigned char requestID;
	unsigned char operation;
	unsigned long result;
} __attribute__((__packed__));
typedef struct received received_t;


int cLength(char *msg) {
	int consonants = 0;
	int tml = msg[0] - '0';
	for(int i = 3; i < tml; i++) {
		char c = msg[i];

		printf("This is c: %c\n",c); 

		if(isalpha(c)){
			switch(c){
				case 'A':
				case 'a':
				case 'E':
				case 'e':
				case 'I':
				case 'i':
				case 'O':
				case 'o':
				case 'U':
				case 'u':
				case 'Y':
				case 'y':
					break;
				default:
					consonants++;
					break;
			}
		} 	 
	}//end of for loop
	
	return consonants; 		
}

int numvowels(char *msg) {
	int vowels = 0;
	int tml = msg[0] - '0'; 
	
	for(int i = 3; i < tml; i++) {
		char c = msg[i];
		if(isalpha(c)){
			switch(c){
				case 'A':
				case 'a':
				case 'E':
				case 'e':
				case 'I':
				case 'i':
				case 'O':
				case 'o':
				case 'U':
				case 'u':
				case 'Y':
				case 'y':
					vowels++;
					break;
				default:
					break;
			}
		} 	 
	}//end of for loop
	
	return vowels; 		
}

message_t disemvowel(char *msg) {
	int tml = msg[0] - '0'; 
	int numvowel = numvowels(msg);
	int length = tml - numvowel - 1;  
	char *result = (char *) malloc(sizeof(char) * length); 
	
	result[0] = length;
	result[1] = msg[1] - '0'; 
	int location = 2;
	for(int i = 3; i < tml; i++){
		char c = msg[i]; 
		if(isalpha(c)){
			switch(c){
				case 'A':
				case 'a':
				case 'E':
				case 'e':
				case 'I':
				case 'i':
				case 'O':
				case 'o':
				case 'U':
				case 'u':
				case 'Y':
				case 'y':
					break;
				default:
					result[location] = c; 
					location++;
					break;
			}
		}
		else {
			result[location] = c;
			location++;
		}
	}
	
	message_t message; 
	message.tml = htons(result[0]);
	message.requestID = htons(result[1]);

	unsigned long l = 0;
	for (int i = 3; i < tml; ++i) {
		l = l | ((unsigned long) result[i] << (8*i)); 
	}
	message.result = htonl(l);	
	return message;
	//return result;
}

message_t uppercase(char *msg) {
	int tml = msg[0] - '0';
	for(int i = 3; i < tml; i++){
		char c = msg[i];
		if(islower(c)){
			msg[i] = toupper(c); 
		}
	
	}
	
	message_t message; 
	message.tml = htons(msg[0]);
	message.requestID = htons(msg[1]);

	unsigned long l = 0;
	for (int i = 3; i < tml; ++i) {
		l = l | ((unsigned long) msg[i] << (8*i)); 
	}
	message.result = htonl(l);	
	return message;

//	return msg; 
}


int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	char buf[MAXDATASIZE];
	int byte_count; 		


	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener

			//this is the message receiving section			
			//char buf[256];
			//int byte_count; 
		
				
			if((byte_count = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1)
			{
				perror("recv"); 
				exit(1);
			}
			buf[byte_count] = '\0';
			
			printf("Here is what I received:%s\n ",buf); 			
			printf("This is %d bytes\n", byte_count); 		
			printf("TML: %d\n", int(buf[0]));
		
			// message handling
			int tml = buf[0] - '0'; 
			int request_id = buf[1] - '0';
			int operation = buf[2] - '0'; 
			//int op = int(operation);
			
			printf("This is TML: %d, RID: %d, OP: %d\n", tml, request_id, operation);



			for(int i = 0; i < 9; i++) {
				char x = buf[i];
				std::cout << x;
			}	
	
			switch(operation) {
				case 5: //cLength
					{
						int consonants = cLength(buf);
						char msg[3];
						msg[0] = 3;
						msg[1] = request_id;
						msg[2] = consonants; 
						//message_t msg;
						//msg.tml = '3';
						//msg.requestID = request_id;
						//msg.result = consonants;	
						printf("Numconsonants: %d\n", consonants);
						if(send(new_fd, &msg, 3, 0) == -1)
							perror("send");
					} 
					break;
				case 80: //disemvoweling
					{	
						message_t msg = disemvowel(buf);
						if(send(new_fd, &msg, msg.tml, 0) == -1)
							perror("send");
					}
					break;
				case 10: //uppercasing 
					{
						message_t msg = uppercase(buf);
						if(send(new_fd, &msg, msg.tml, 0) == -1)
							perror("send");  
					}	
					break;
				default:
					break; 
			}				

			//if (send(new_fd, "Hello, world!", 13, 0) == -1)
			//	perror("send");

			printf("\nat the end of the big if");
			close(new_fd);
			exit(0);
		}
		printf("I am skipping the if"); 
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}	
