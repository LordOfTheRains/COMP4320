#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "ChatClient.h"



int configure(ChatClient* client_ptr, char* server_ip, int server_port, int my_port){
    client_ptr->myPort = my_port;
    if(configureServerSocket(client_ptr,server_ip, server_port)){
        perror("configure server socket failed");
        return -1;
    }
    if(configureMySocket(client_ptr, my_port)){
        perror("configure my socket failed");
        return -1;
    }
    return 0;
}

int configureMySocket(ChatClient* client_ptr, int my_port){
    client_ptr->mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_ptr->mySocket == -1){
        printf("Error opening socket\n");
        return -1;
    }

    client_ptr->myAddr.sin_port = htons(my_port);
    client_ptr->myAddr.sin_addr.s_addr = 0;
    client_ptr->myAddr.sin_addr.s_addr = INADDR_ANY;
    client_ptr->myAddr.sin_family = AF_INET;
    if(bind(client_ptr->mySocket, (struct sockaddr *) &(client_ptr->myAddr),
        sizeof(client_ptr->myAddr) ) == -1) {
        printf("Error binding my socket\n");
        return -1;
    }
    return 0;
}


int configureServerSocket(ChatClient* client_ptr, char* server_ip, int server_port){
    client_ptr->serverAddr.sin_family = AF_INET;
    client_ptr->serverAddr.sin_port = htons(server_port);
    client_ptr->serverAddr.sin_addr.s_addr = inet_addr(server_ip);
    if ((client_ptr->serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create server socket");
    }
    return 0;
}

int sentChatRequest(ChatClient* client_ptr, ChatRequest* req, void* response){
    int res_num_byte;
    if (sendto(client_ptr->serverSocket, req,
            sizeof(ChatRequest), 0, (struct sockaddr *) &(client_ptr->serverAddr),
            sizeof(struct sockaddr_in)) < 0) {
        perror("sendto failed");
        return 0;
    }
    puts("waiting for server response\n");
    res_num_byte = recvfrom(client_ptr->serverSocket, response, sizeof(ServerResponse), 0,
                            (struct sockaddr *) &(client_ptr->serverAddr),
                            sizeof(struct sockaddr_in));
    if (res_num_byte < 0)
      perror("ERROR in recvfrom");
    printf("%.*s",res_num_byte,response);
    return 0;
}

//connect to partner if server provides ip
//if no partner then listen on own port
void run(ChatClient* client_ptr){
    ServerResponse response;
    ChatRequest req = getChatRequest(client_ptr);
    sentChatRequest(client_ptr,&req, &response);
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
