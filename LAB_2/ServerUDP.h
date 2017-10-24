
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
      uint32_t magicNumber;
      uint16_t tml;
      unsigned char GID;
      unsigned char checksum;
      unsigned char requestID;
      char* hostList;
      unsigned char error;
    };

    struct ValidResponse{
        uint32_t  magicNumber;
        uint16_t  tml;
        unsigned char GID;
        unsigned char checksum;
        unsigned char requestID;
    	char* ipAddresses;
    } __attribute__((__packed__));

    struct InvalidResponse{
        uint32_t  magicNumber;
        uint16_t tml;
        unsigned char GID;
        unsigned char checksum;
        unsigned char errorCode;
    } __attribute__((__packed__));
  private:

    int sock; //PORT
    int port;

    void processRaw(char *msg, size_t num_byte, struct ClientRequest* result);
    ValidResponse getResponse(ClientRequest *req);
    char getChecksum(char* msg, int num_bytes);
    string resolveHostnames(char* msg, int num_bytes);
    unsigned long toBinary(string msg);
    void display(char *Buffer, int length);
};


#endif
