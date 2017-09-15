
#ifndef SERVER_UDP_H
#define SERVER_UDP_H



#include <string>

class ServerUDP {

  struct ClientRequest{
    int msgLength;
    int requestID;
    int operation;
    string message;
    int error;
  };


  public:
    ServerUDP(int port);
    int configure();
    void run();

  private:

    int sock; //PORT
    int port;


    ClientRequest processRaw(string msg);
    string getResponse(std::msg);
    string getCLength(string:msg);
    string disemvoweling(string:msg);
    string upperCasing(string:msg);
    void display(char *Buffer, int length);
};


#endif
