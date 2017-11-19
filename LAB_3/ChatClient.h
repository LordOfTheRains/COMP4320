#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H


struct ChatRequest{
  uint32_t magicNumber;
  uint16_t tml;
  unsigned char GID;
  uint32_t port;
};

struct ServerResponse{
    uint32_t  magicNumber;
    uint16_t  tml;
    unsigned char GID;
    unsigned char checksum;
    unsigned char requestID;
} __attribute__((__packed__));


struct ChatClient {
    struct sockaddr_in myAddr;
    int myPort;
    int mySocket; //port it listens on for chat connection
    int serverSocket; //Server socket
    struct sockaddr_in serverAddr;
    int partnerSocket;
    int partnerPort;
};


typedef struct ServerResponse ServerResponse;
typedef struct ChatRequest ChatRequest;
typedef struct ChatClient ChatClient;

//configure creates the udp connection to allow client to talk to server
int configure(ChatClient* client_ptr, char* server_ip, int port, int my_port);

//configure client tcp socket used to chat
int configureMySocket(ChatClient* client_ptr, int my_port);

//configure socket used to connect to server
int configureServerSocket(ChatClient* client_ptr, char* server_ip, int server_port);

//sends chat request to udp server
int sentChatRequest(ChatClient* client_ptr, ChatRequest* req, void* response);

//connect to partner if server provides ip
//if no partner then listen on own port
void run(ChatClient* client_ptr);
//creates a chat request from clieent port/ip
ChatRequest getChatRequest(ChatClient* client_ptr);

//process the raw response from server, print processed data
void processRaw(size_t num_bytes, struct ServerResponse* res);


//creates a tcp connection to the client
void connectPartner(ChatClient* client_ptr,char* partner_ip, int port);

//listens to the partner socket and start chatting (case where client gets a partner)
void initiateChat(ChatClient* client_ptr);

//listens on own port for partner (case where client registered to chat and waits f
// for an partner to connect)
void waitToChat(ChatClient* client_ptr);


void display(char *Buffer, int length);

#endif
