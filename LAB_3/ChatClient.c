#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "ChatClient.h"


int configure(ChatClient* client_ptr, char* server_ip, int port, int my_port){
    if ((client_ptr->serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    	perror("cannot create socket");
    }
    client_ptr->serverPort = port;
    client_ptr->myPort = my_port;
    struct sockaddr_in server;
    int err;
    server.sin_family = AF_INET;
    server.sin_port = htons(client_ptr->serverPort);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    err = bind(client_ptr->serverSocket, (struct sockaddr *) &server, sizeof(server));
    if (err < 0){
      puts("Could not bind socket\n");
      return 1;
    }
    //setup socket and bind
    puts("server socket binded...\n");
    return 0;
}

//connect to partner if server provides ip
//if no partner then listen on own port
void run(ChatClient* client_ptr){
    return;
}

//creates a chat request from clieent port/ip
ChatRequest getChatRequest(ChatClient* client_ptr){
    ChatRequest req;
    req.magicNumber = htonl(0x4a6f7921);
    req.GID = 7;
    req.port = client_ptr->myPort;
    return req;
}



//process the raw response from server, print processed data
void processRaw(size_t num_bytes, ServerResponse* req){

    return;
}


void display(char *Buffer, int length){
    int currentByte, column;

    currentByte = 0;
    puts("\n>>>>>>>>>>>> Content in hexadecimal <<<<<<<<<<<\n");
    while (currentByte < length){
      printf("%3d: ", currentByte);
      column =0;
      while ((currentByte < length) && (column < 10)){
        printf("%2x ",Buffer[currentByte]);
        column++;
        currentByte++;
      }
      printf("\n");
     }
     printf("\n\n");
     return;
}
