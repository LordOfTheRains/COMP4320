
#ifndef SERVER_UDP_H
#define SERVER_UDP_H



#include <string>

using namespace std;

class ServerUDP {




  public:
    ServerUDP(int port);
    int configure();
    void run();


    struct ClientRequest{
      unsigned long magicNumber;
      unsigned short tml;
      unsigned char GID;
      unsigned char checksum;
      unsigned char requestID;
      char* hostInfo;
      int error;
    };

    struct ValidResponse{
        unsigned long magicNumber;
        unsigned short tml;
        unsigned char GID;
        unsigned char checksum;
        unsigned char requestID;
    	char* ipAddresses;
    } __attribute__((__packed__)) validResponse;

    struct InvalidResponse{
        unsigned long magicNumber;
        unsigned short tml;
        unsigned char GID;
        unsigned char checksum;
        unsigned char errorCode;
    } __attribute__((__packed__)) invalidResponse;
  private:

    int sock; //PORT
    int port;

    ClientRequest processRaw(char *msg, size_t num_byte);
    ValidResponse getResponse(ClientRequest *req);
    char getChecksum(char* msg);
    void resolveHostnames(char* msg, const  char* ipAddrs);
    unsigned long toBinary(string msg);
    void display(char *Buffer, int length);
};


#endif
