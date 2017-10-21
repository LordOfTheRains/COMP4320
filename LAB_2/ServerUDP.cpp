#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <typeinfo>
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
      processRaw(buffer, num_byte,req);

      if (req.error){
        printf("\n >>>> invalid request:...\n");
        InvalidResponse resp;
        resp.magicNumber = htonl(0x4a6f7921);
        resp.tml = htons(9);
        resp.GID = 7;
        resp.checksum = 0;
        resp.errorCode = req.error;
        sendto(this->sock,&resp,9,0,(struct sockaddr *)&client,sizeof(client));
        //add the other info for a invalid request
      }else{
        printf("\n >>>> processing request:...\n");
        ValidResponse *res = NULL;
        string ipstr = resolveHostnames(req.hostInfo, req.tml-9);

        printf("ok.\n");
        res = (struct ValidResponse *)malloc(sizeof(struct ValidResponse) + ipstr.length());
        res->magicNumber = htonl(0x4a6f7921);
        res->tml = htons(10);
        res->GID = 7;
        res->checksum = 0;
        res->requestID = req.requestID;
        memcpy(&res->ipAddresses, ipstr.c_str(), ipstr.length());
        res->tml = htons(ipstr.length() + 9);
        size_t struct_total_length = sizeof (ValidResponse) + ipstr.length();
        char datagram[struct_total_length] = {0};
        memcpy(datagram, res, struct_total_length);

        printf("sizeof packed_struct: %ld \n", struct_total_length);
        printf("----------------Packet Content ---------------\n");
        display(datagram, sizeof(datagram));
        res->checksum = getChecksum(datagram, sizeof(datagram));
        printf("\nsending response: %ld bytes\n",sizeof(res));
        //then send the response message back to sender;
        sendto(this->sock,res,struct_total_length,0,(struct sockaddr *)&client,sizeof(client));
        printf("response sent.\n");
        delete(res);
      }


    }
  }
  close(this->sock);
}

void ServerUDP::processRaw(char *buffer, size_t num_byte, ClientRequest& result){
  printf("\n------------- parsing raw data ---------------------\n");
  memcpy(&result.magicNumber, &buffer[0], sizeof(result.magicNumber));
  result.magicNumber = ntohl(result.magicNumber);
  memcpy(&result.tml, &buffer[4], sizeof(result.tml));
  result.tml = ntohs(result.tml);
  memcpy(&result.GID, &buffer[6], sizeof(result.GID));
  memcpy(&result.checksum, &buffer[7], sizeof(result.checksum));
  memcpy(&result.requestID, &buffer[8], sizeof(result.requestID));
  char hosts[ result.tml-9];
  memcpy(hosts, &buffer[9], result.tml-9);
  strcpy(result.hostInfo, hosts);
  result.error = 0b0000;
  if (result.magicNumber != 0x4a6f7921){
      result.error = result.error | 0b0001;
      printf("magic number error \n");
  }
  if (result.checksum != getChecksum(buffer, int(num_byte)) ){
      result.error = result.error | 0b0100;
      printf("checksum error \n");
  }
  if (result.tml != num_byte) {
      result.error = result.error | 0b0001;
      printf("tml error \n");
  }
  free(hosts);
  printf("\n------------- parsing raw data completed ---------------------\n");
  return;
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
  cout <<  res.ipAddresses << endl;
  return res;

}

//returns cmputed checksum
char ServerUDP::getChecksum(char* msg, int num_bytes){
  printf("\n--------- Compute Checksum --------- \n");
  int currentByte = 0;
  while (currentByte < num_bytes){
    printf("%2x: ",  msg[currentByte]);
    currentByte++;
  }
  // do checksum magic

  printf("\n--------- Compute Checksum --------- \n");
  return 0;
}
//return a byte?string of ip address from a string of hosts
//input might be: "10google.com12facebook.com"
string ServerUDP::resolveHostnames(char* msg, int num_bytes){
  printf("\n---------resolving host names-----------\n");
  //parse the message to get list of host names off
  //call hostent * ip = gethostbyname("google.com");
  int currentByte = 0;
  while (currentByte < num_bytes){
    printf("%2x: ", msg[currentByte]);
    currentByte++;
  }
  printf("\n---------host names resolved--------------\n");
  return "hellno";
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
