#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <cstdlib>           // For atoi()
#include <stdexcept>
#include <unistd.h>
#include<iostream>
#include <errno.h>
#include <stdio.h>
#include <cstring>
#include <ctype.h>
#include <ServerUDP.h>
#include <bitset>

#include <algorithm>
#include <climits>
using namespace std;


#define BUF_SIZE 1024

ServerUDP::ServerUDP(int port){
  if ((this->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
  	perror("cannot create socket");
  }
  this->port = port;
}

int ServerUDP::configure(){
  struct sockaddr_in server;
  int err;
  server.sin_family = AF_INET;
  server.sin_port = htons(this->port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  err = bind(this->sock, (struct sockaddr *) &server, sizeof(server));
  if (err < 0){
    puts("Could not bind socket\n");
    return 1;
  }
  //setup socket and bind
  puts("server socket binded...\n");
  return 0;
}

void ServerUDP::run(){

    int num_byte;
    socklen_t addr_len;
    char buffer[BUF_SIZE];
    struct sockaddr_in client;
// run the server continuously.
//not sure if i need to fork for udp if new connection detected.
  while (1) {
    addr_len = sizeof(client);
    /* read a datagram from the socket (put result in bufin) */
    num_byte=recvfrom(this->sock,buffer,BUF_SIZE,0,(struct sockaddr *)&client,&addr_len);

    /* print out the address of the sender */
    printf("Got a datagram from %s port %d\n",
           inet_ntoa(client.sin_addr), ntohs(client.sin_port));

    if (num_byte<0) {
      perror("Error receiving data");
    } else {
      printf("GOT %d BYTES\n",num_byte);
      display(buffer,num_byte);
      // handle this packet
      ClientRequest req;// = processRaw(buffer);
      req = processRaw(buffer, num_byte);
      if (req.error){
        printf("\n >>>> invalid request:...\n");
        InvalidResponse resp;
        resp.checksum = 0;

        sendto(this->sock,&resp,sizeof(resp),0,(struct sockaddr *)&client,sizeof(client));
        //add the other info for a invalid request
      }else{
        printf("\n >>>> processing request:...\n");
        ValidResponse resp = getResponse(&req);
        //printf("\nsending response: %llu\n",resp.ipAddresses);
        printf("response size: %lu\n",sizeof(resp));
        //then send the response message back to sender;
        sendto(this->sock,&resp,sizeof(resp),0,(struct sockaddr *)&client,sizeof(client));
        char respchar[sizeof(ValidResponse)];
        memcpy(respchar, &resp, sizeof(ValidResponse));
        display(respchar,(int)sizeof(resp));
        printf("response sent.\n");
      }
      /* Got something, just send it back */

    }
  }
  close(this->sock);
}

ServerUDP::ClientRequest ServerUDP::processRaw(char *buffer, size_t num_byte){
  ClientRequest req;
  memcpy(&req.magicNumber, &buffer[0], sizeof(req.magicNumber));
  req.magicNumber = ntohl(req.magicNumber);
  memcpy(&req.tml, &buffer[4], sizeof(req.tml));
  req.tml = ntohs(req.tml);
  memcpy(&req.GID, &buffer[6], sizeof(req.GID));
  memcpy(&req.checksum, &buffer[7], sizeof(req.checksum));
  memcpy(&req.requestID, &buffer[8], sizeof(req.requestID));
  memcpy(&req.hostInfo, &buffer[9], sizeof(req.hostInfo));

  req.error = 0b0000;
  if (req.magicNumber != 0x4a6f7921){
      req.error = req.error | 0b0001;
      printf("magic number error \n");
  }
  if (req.checksum != getChecksum(buffer) ){
      req.error = req.error | 0b0100;
      printf("checksum error \n");
  }
  if (req.tml != num_byte) {
      req.error = req.error | 0b0001;
      printf("tml error \n");
  }
  return req;
}


// returns the final message ready to be sent back to client
ServerUDP::ValidResponse ServerUDP::getResponse(ClientRequest *req){
  //pack the result message
  ValidResponse res;
  res.magicNumber = htonl(0x4a6f7921);
  res.tml = htons(10);
  res.GID = 7;
  res.checksum = 0;
  res.requestID = req->requestID;
  resolveHostnames(req->hostInfo, res.ipAddresses);
  res.tml = htons(sizeof(res.ipAddresses) + 9);
  char datagram[sizeof(res)];
  memcpy(datagram, &res, sizeof(datagram));
  res.checksum = getChecksum(datagram);
  return res;

}

//returns cmputed checksum
char ServerUDP::getChecksum(char* msg){
  // do checksum magic
  return 0;
}
//return a byte?string of ip address from a string of hosts
//input might be: "10google.com12facebook.com"
void ServerUDP::resolveHostnames(char* msg, const  char* ipAddrs){
  //parse the message to get list of host names off
  //call hostent * ip = gethostbyname("google.com");
  string result = "hello";
  ipAddrs = result.c_str();
  return;
}


unsigned long ServerUDP::toBinary(string msg){
    //const size_t MAX = sizeof(unsigned long long);
    reverse(msg.begin(),msg.end());
    unsigned long long resbin = 0;

    //for (size_t i=0; i < std::min(MAX, msg.size()); ++i)
    for (size_t i=0; i < msg.size(); ++i)
    {
      resbin <<= CHAR_BIT;
      resbin += (unsigned char) msg[i];
    }
    std::cout << std::hex << resbin;
    return resbin;
}

void ServerUDP::display(char *Buffer, int length){
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
