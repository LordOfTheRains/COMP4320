#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H


struct ChatRequest{
  uint32_t magicNumber;
  uint16_t port;
  unsigned char GID;
}__attribute__((__packed__));

struct ServerResponse{
    uint32_t magicNumber;
    uint32_t waitingClientIP;
    uint16_t waitingClientPort;
    unsigned char GID;
}__attribute__((__packed__));

struct ServerResponseNoWait{
    uint32_t magicNumber;
    unsigned char GID;
    uint16_t myPort;
}__attribute__((__packed__));


struct ChatClient {
    struct sockaddr_in myAddr;
    int myPort;
    int mySocket; //port it listens on for chat connection
    struct sockaddr_in serverAddr;
    int serverSocket; //Server socket
    struct sockaddr_in partnerAddr;
    int partnerSocket;
};


typedef struct ServerResponse ServerResponse;
typedef struct ServerResponseNoWait ServerResponseNoWait;
typedef struct ChatRequest ChatRequest;
typedef struct ChatClient ChatClient;

//configure creates the udp connection to allow client to talk to server
int configure(ChatClient* client_ptr, char* server_ip, int port, int my_port);

//configure  tcp socket used to chat
int configureMySocket(ChatClient* client_ptr);

int configurePartnerSocket(ChatClient* client_ptr, uint32_t partner_ip, int partner_port);

//configure socket used to connect to server
int configureServerSocket(ChatClient* client_ptr, char* server_ip, int server_port);

//sends chat request to udp server
int sendChatRequest(ChatClient* client_ptr, ChatRequest* req, void* response, int *numbytes);

//connect to partner if server provides ip
//if no partner then listen on own port
void run(ChatClient* client_ptr);
//creates a chat request from clieent port/ip
ChatRequest getChatRequest(ChatClient* client_ptr);

//process the raw response from server for registered request
void processRegisteredResponse(void* data, ServerResponseNoWait* noWaitResponse);

void processWaitClientResponse(void* data, ServerResponse* response);

//creates a tcp connection to the client
void connectPartner(ChatClient* client_ptr,char* partner_ip, int port);

//listens to the partner socket and start chatting (case where client gets a partner)
void initiateChat(ChatClient* client_ptr,uint32_t partner_ip, int partner_port);

//listens on own port for partner (case where client registered to chat and waits f
// for an partner to connect)
void waitToChat(ChatClient* client_ptr);


void display(char *Buffer, int length);

#endif
