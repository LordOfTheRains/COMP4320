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


#define BUF_SIZE 4096

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
    ClientRequest clientReq;
    struct sockaddr_in client;
// run the server continuously.
//not sure if i need to fork for udp if new connection detected.
  while (1) {
    addr_len = sizeof(client);
    /* read a datagram from the socket (put result in bufin) */
    num_byte=recvfrom(this->sock,&clientReq,sizeof(clientReq),0,(struct sockaddr *)&client,&addr_len);

    /* print out the address of the sender */

    printf("Got a datagram from %s port %d\n",
           inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    printf("-----Unadjusted byte order packet content-----\n");
    printf("Magic: %08x\nTML: %04x \nGID: %2x\nchecksum: %2x\nrequestID: %2x\n",
                                    clientReq.magicNumber,
                                    clientReq.tml,
                                    clientReq.GID,
                                    clientReq.checksum,
                                    clientReq.requestID);
    if (num_byte<0) {
      perror("Error receiving data");
    } else {
      printf("GOT %d BYTES\n",num_byte);
      processRaw(num_byte, &clientReq);
      if (clientReq.error){
        printf("\n >>>> invalid request:...\n");
        InvalidResponse resp;
        resp.magicNumber = htonl(0x4a6f7921);
        resp.tml = htons(9);
        resp.GID = 7;
        resp.checksum = 0x0;
        resp.errorCode = clientReq.error;
        printf("----------------Invalid Response Content (%d bytes)---------------\n", 9);
        resp.checksum = getChecksum(&resp, sizeof(resp));
        for(int j = 0; j < 9; ++j)
          printf("%02x ", ((uint8_t*) &resp)[j]);
        sendto(this->sock,&resp,9,0,(struct sockaddr *)&client,sizeof(client));
        printf("Invalid response sent.\n");
      }else{
        printf("\n >>>> processing request:...\n");
        ValidResponse res;
        size_t ip_bytes = resolveHostnames(clientReq.hostList, clientReq.tml-9, res.ipAddresses);
        int packetSize = ip_bytes + 9;
        res.magicNumber = htonl(0x4a6f7921);
        res.tml = htons(packetSize);
        res.GID = 7;
        res.checksum = 0x0;
        res.requestID = clientReq.requestID;
        printf("----------------Packet Content(%d bytes) --------------- \n", packetSize);
        res.checksum = getChecksum(&res, packetSize);
        for(int j = 0; j < packetSize; ++j)
          printf("%02x ", ((uint8_t*) &res)[j]);
        printf("checking checksum: %02x", getChecksum(&res, packetSize));
        printf("\nponse: %d bytes\n",ntohs(res.tml));
        //then send the response message back to sender;
        sendto(this->sock,&res,packetSize,0,(struct sockaddr *)&client,sizeof(client));
        printf("response sent.\n");
      }


    }
  }
  close(this->sock);
}

void ServerUDP::processRaw(size_t num_bytes,struct ClientRequest* result){
  printf("\n------------- parsing raw data ---------------------\n");
  result->magicNumber = ntohl(result->magicNumber);
  result->tml = ntohs(result->tml);
  result->error = 0b0000;
  printf("------ Byte order adjusted packet content------------- \n");
  printf("Magic: %08x\nTML: %04x \nGID: %2x\nchecksum: %2x\nrequestID: %2x\n",
                                  result->magicNumber,
                                  result->tml,
                                  result->GID,
                                  result->checksum,
                                  result->requestID);
  if (result->magicNumber != 0x4a6f7921){
      result->error = result->error | 0b0001;
      printf("magic number error \n");
  }
  if (getChecksum(result, int(num_bytes)) != 0){
      result->error = result->error | 0b0100;
      printf("checksum error \n");
  }
  if (result->tml != num_bytes) {
      result->error = result->error | 0b0001;
      printf("tml error \n");
  }


  printf("\n------------- parsing raw data completed ---------------------\n");
  return;
}


//not being used
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
unsigned char ServerUDP::getChecksum(void* msg, int num_bytes){
    printf("\n--------- Compute Checksum --------- \n");
    char* check_data = (char*)msg;
    // do checksum magic
    //assume the checksum is set to 0 prior to this function
    //sum all bytes
    int currentByte = 0;
  while (currentByte < num_bytes){
    printf("%2x: ",  (unsigned char)check_data[currentByte]);
    currentByte++;
  }
    int currentSum = 0;
    for(int i = 0; i < num_bytes; i++){
    	currentSum += (unsigned char) check_data[i];
    	//handle carry
    	if(currentSum > 255){
    		currentSum = currentSum - 256 + 1;
    	}
    }
    //print sum
    printf("\nSum result: %d\n", currentSum);
    //bitwise one complement of sum
    int compSum = (int) ~currentSum & 0xff;
    unsigned char finalSum = (unsigned char) compSum;
    printf("checksum: %2x: ",  finalSum);
    printf("\n--------- Compute Checksum completed --------- \n");
    return finalSum;
}
//return a byte?string of ip address from a string of hosts
//input might be: "10google.com12facebook.com"
size_t ServerUDP::resolveHostnames(char* msg, int num_bytes, void* container){
  printf("\n---------resolving host names-----------\n");
  int currentByte = 0;
  int total_bytes = 0;
  int num_ips = 0;
  uint32_t ips[50] = {0};
  while (currentByte < num_bytes){
      int host_size = 0;
      memcpy(&host_size, &msg[currentByte], 1);
      printf("current host_size is: %d: \n", host_size);
      char hostname[host_size+1];
      memcpy(&hostname, &msg[currentByte+1], host_size);
      hostname[host_size] = '\0'; //null terminating the string
      //display(hostname, sizeof(hostname));
      struct hostent *hp = gethostbyname(hostname);
      struct in_addr *ip_addr;
        if (hp == NULL) {
           printf("gethostbyname() failed\n");
           ips[num_ips] = 0xffffffff;
           printf("%8x = ", ips[num_ips]);
           num_ips++;
           total_bytes+=4;
        } else {
           printf("%s = ", hp->h_name);
           if ( hp -> h_addr_list[0] != NULL) {
               ip_addr = ( struct in_addr*)( hp -> h_addr_list[0]);
               char* ip = inet_ntoa( *( struct in_addr*)( hp -> h_addr_list[0]));
               ips[num_ips] = ip_addr->s_addr;
               printf("%8x = ", ips[num_ips]);
               printf( "[%s]\n",ip );
               num_ips++;
               total_bytes+=4;
           }
        }
      currentByte+= host_size+1;
  }
  printf("total_bytes: %d\n",total_bytes);
  memcpy(container, &ips, total_bytes);

  printf("\n---------host names resolved--------------\n");
  return total_bytes;
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
