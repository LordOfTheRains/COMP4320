#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "ChatClient.h"


int configure(ChatClient* client_ptr, char* server_ip, int server_port, int my_port){
    client_ptr->myPort = my_port;
    if(configureServerSocket(client_ptr,server_ip, server_port)){
        perror("configure server socket failed");
        return -1;
    }
    return 0;
}


int configureMySocket(ChatClient* client_ptr){
    client_ptr->mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_ptr->mySocket == -1){
        printf("Error opening socket\n");
        return -1;
    }
    client_ptr->myAddr.sin_port = htons(client_ptr->myPort);
    client_ptr->myAddr.sin_addr.s_addr = INADDR_ANY;
    client_ptr->myAddr.sin_family = AF_INET;
    if(bind(client_ptr->mySocket, (struct sockaddr *) &(client_ptr->myAddr),
        sizeof(client_ptr->myAddr) ) == -1) {
        printf("Error binding my socket\n");
        return -1;
    }
    return 0;
}

int configurePartnerSocket(ChatClient* client_ptr, uint32_t partner_ip, int partner_port){
    client_ptr->partnerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_ptr->partnerSocket == -1){
        printf("Error opening socket\n");
        return -1;
    }

    client_ptr->partnerAddr.sin_port = htons(partner_port);
    client_ptr->partnerAddr.sin_addr.s_addr = htonl(partner_ip);
    client_ptr->partnerAddr.sin_family = AF_INET;
    if(connect(client_ptr->partnerSocket, (struct sockaddr *) &(client_ptr->partnerAddr),
        sizeof(client_ptr->partnerAddr) ) == -1) {
        printf("Error connecting my socket\n");
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

int sendChatRequest(ChatClient* client_ptr, ChatRequest* req, void* response, int *numbytes){
    socklen_t fromlen;
    fromlen = sizeof(struct sockaddr_storage);
    if (sendto(client_ptr->serverSocket, req,
            sizeof(ChatRequest), 0, (struct sockaddr *) &(client_ptr->serverAddr),
            sizeof(struct sockaddr_in)) < 0) {
        perror("sendto failed");
        return 0;
    }
    puts("waiting for server response\n");
    size_t bytes = recvfrom(client_ptr->serverSocket, response, sizeof(struct ServerResponse), 0,
                            (struct sockaddr *) &client_ptr->serverAddr,
                            &fromlen);
    *numbytes = bytes;
    if (bytes < 0)
      perror("ERROR in recvfrom");
    display(response,bytes);
    return 0;
}

//connect to partner if server provides ip
//if no partner then listen on own port
void run(ChatClient* client_ptr){
    ServerResponseNoWait noWaitResponse;
    ServerResponse response;
    char* buff[2048];
    int res_size;
    int* size_ptr = &res_size;
    ChatRequest req = getChatRequest(client_ptr);
    sendChatRequest(client_ptr,&req,&buff,size_ptr);
    if (res_size ==7){
        processRegisteredResponse(&buff, &noWaitResponse);
        printf("Magic: %08x\nGID: %02x \nwaitingClientPort: %04x\n",
                                        noWaitResponse.magicNumber,
                                        noWaitResponse.GID,
                                        noWaitResponse.myPort);
        waitToChat(client_ptr);
    }else{
        processWaitClientResponse(&buff, &response);
        printf("Magic: %08x\nwaitingClientIP: %08x \nwaitingClientPort: %4x\nGID: %2x\n",
                                        response.magicNumber,
                                        response.waitingClientIP,
                                        response.waitingClientPort,
                                        response.GID);
        initiateChat(client_ptr,response.waitingClientIP, response.waitingClientPort);
    }
    return;
}
void waitToChat(ChatClient* client_ptr){
    puts("waiting for a chat partner...");
    int partnerSock;
    struct sockaddr_in partner;
    char myMsg[256],msg[256];

    if(configureMySocket(client_ptr)){
        perror("configure my socket failed");
        return;
    }
    while(1){
        if (listen(client_ptr->mySocket,SOMAXCONN)) {
            perror("failed to listen for connections");
            return;
        }
        socklen_t partner_len = sizeof(partner);
        partnerSock = accept(client_ptr->mySocket, (struct sockaddr *) &partner, &partner_len);

        if (partnerSock < 0) {
            perror("Could not establish new connection to connecting partner\n");
            return;
        }
        while (1) {
            int read = recv(partnerSock, &msg, 256, 0);
            printf("New message: %s\n",msg);
            printf("please enter message:\n");
        //scanf("%s", &name);  - deprecated
            fgets(myMsg,256,stdin);
            if (sendto(partnerSock, &myMsg,
                    sizeof(myMsg), 0, (struct sockaddr *) &(partner),
                    sizeof(struct sockaddr_in)) < 0) {
                perror("sendto failed");
                return;
            }
        }
    }
}

void initiateChat(ChatClient* client_ptr,uint32_t partner_ip, int partner_port){
    puts("connecting to chat partner...");
    if(configurePartnerSocket(client_ptr,partner_ip, partner_port)){
        perror("configure partner socket failed");
        return;
    }
    char myMsg[256], msg[256];
    while(1){
        printf("please enter message:\n");
    //scanf("%s", &name);  - deprecated
        fgets(myMsg,256,stdin);
        if (sendto(client_ptr->partnerSocket, &myMsg,
                sizeof(myMsg), 0, (struct sockaddr *) &(client_ptr->partnerAddr),
                sizeof(struct sockaddr_in)) < 0) {
            perror("sendto failed");
            return;
        }
        int read = recv(client_ptr->partnerSocket, &msg, 256, 0);
        printf("New message: %s\n",msg);
    }
}


//creates a chat request from clieent port/ip
ChatRequest getChatRequest(ChatClient* client_ptr){
    ChatRequest req;
    req.magicNumber = htonl(0x4a6f7921);
    req.GID = 7;
    req.port = htons(client_ptr->myPort);
    return req;
}

//process the raw response from server for registered request
void processRegisteredResponse(void* data, ServerResponseNoWait* noWaitResponse){
    *noWaitResponse = *(struct ServerResponseNoWait *)data;
    noWaitResponse->magicNumber = ntohl(noWaitResponse->magicNumber);
    noWaitResponse->myPort = ntohs(noWaitResponse->myPort);
    return;
}

void processWaitClientResponse(void* data, ServerResponse* response){
    *response = *(struct ServerResponse *)data;
    response->magicNumber = ntohl(response->magicNumber);
    response->waitingClientIP = ntohl(response->waitingClientIP);
    response->waitingClientPort = ntohs(response->waitingClientPort);
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
