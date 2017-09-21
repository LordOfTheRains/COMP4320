
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
      int msgLength;
      int requestID;
      int operation;
      string message;
      int error;
    };

  private:

    int sock; //PORT
    int port;

    ClientRequest processRaw(string msg);
    string getResponse(ClientRequest *req);
    string getCLength(string msg);
    string disemvoweling(string msg);
    string upperCasing(string msg);
    void display(char *Buffer, int length);
};


#endif
