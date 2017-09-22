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
#include <string>

#include <algorithm>
#include <climits>

using namespace std; 
//now in commandline #define PORT "10017"  // the port users will be connecting to

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
	unsigned long long result; 
} __attribute__((__packed__)); 

typedef struct message message_t; 

struct received 
{
	int tml;
	int requestID;
	int operation;
	string message;
	int error;
};
typedef struct received received_t;


int cLength(received_t rec) {
	int consonants = 0;
	for(int i = 0; i < rec.tml-3; i++) {
		char c = rec.message[i];

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

int numvowels(received_t rec) {
	int vowels = 0;
	for(int i = 0; i < rec.tml-3; i++) {
		char c = rec.message[i];
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

unsigned long long toBinary(string msg) {
	const size_t MAX = sizeof(unsigned long long);
	reverse(msg.begin(),msg.end());
	unsigned long long resbin = 0;
	
	for (size_t i=0; i < std::min(MAX, msg.size()); ++i){
		resbin <<= CHAR_BIT;
		resbin += (unsigned char) msg[i];
	}
	std::cout << std::hex << resbin;
	return resbin; 
}

message_t disemvowel(received_t rec) {
	int numvowel = numvowels(rec);
	
	printf("In disemvowel, numvowel: %d\n", numvowel); 

	int length = rec.tml - numvowel - 1;  
	char *result = (char *) malloc(sizeof(char) * (length-1)); 
	
	int location = 0;
	for(int i = 0; i < rec.tml-3; i++){
		char c = rec.message[i]; 
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
	message.tml = length;
	message.requestID = rec.requestID;
	
	for(int i = 0; i < length-2; i++){
		printf("This is char: %c\n", result[i]);
	}
	
	//unsigned long l = 0;
	//for (int i = 0; i < length-2; ++i) {
	//	l = l | ((unsigned long) result[i] << (8*i)); 
	//}
	//message.result = l;
	result[length] = '\0';
	string str(result); 
	printf("This is the converted result to str: %s\n", str.c_str());
	
	message.result = toBinary(str);	
	return message;
}

message_t uppercase(received_t rec) {
	for(int i = 0; i < rec.tml-3; i++){
		char c = rec.message[i];
		if(islower(c)){
			rec.message[i] = toupper(c); 
		}
	
	}
	
	printf("Result of uppercase: %s\n", rec.message.c_str());	
	message_t message; 
	message.tml = rec.tml;
	message.requestID = rec.requestID;

	message.result = toBinary(rec.message);	
}

received_t processRaw(char *msg){
	received_t rec;
	rec.tml = int(msg[0]);
	rec.requestID = int(msg[1]);
	rec.operation = int(msg[2]);
	string content(&msg[3], &msg[3] + rec.tml);
	rec.message = content;
	rec.error = 0;
	return rec;
}


int main(int argc, char *argv[])
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

	//take in portnumber from commandline
	if (argc != 2) {
		fprintf(stderr, "usage: server portnumber\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
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
		
			if((byte_count = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1)
			{
				perror("recv"); 
				exit(1);
			}
			buf[byte_count] = '\0';
			
			printf("This is %d bytes\n", byte_count); 		
		
			received_t rec = processRaw(buf); 

			int tml = rec.tml; 
			int request_id = rec.requestID;
			int operation = rec.operation;
			

			printf("This is TML: %d, RID: %d, OP: %d\n", tml, request_id, operation);
			printf("This is received message: %s\n", rec.message.c_str());


			switch(operation) {
				case 5: //cLength
					{
						int consonants = cLength(rec);
						message_t msg;
						msg.tml = 3;
						msg.requestID = request_id;
						msg.result = (unsigned long)consonants;	
						printf("Numconsonants: %d\n", consonants);
					
						printf("msg.result: %d\n", msg.result);

						if(send(new_fd, &msg, sizeof(msg), 0) == -1)
							perror("send");
					} 
					break;
				case 80: //disemvoweling
					{	
						message_t msg = disemvowel(rec);
						if(send(new_fd, &msg, sizeof(msg), 0) == -1)
							perror("send");
					}
					break;
				case 10: //uppercasing 
					{
						message_t msg = uppercase(rec);
						if(send(new_fd, &msg, sizeof(msg), 0) == -1)
							perror("send");  
					}	
					break;
				default:
					break; 
			}				

			//if (send(new_fd, "Hello, world!", 13, 0) == -1)
			//	perror("send");

			//printf("\nat the end of the big if");
			close(new_fd);
			exit(0);
		}
		//printf("I am skipping the if"); 
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}	
