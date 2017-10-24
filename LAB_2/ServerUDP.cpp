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
      //display(buffer,num_byte);s
      // handle this packet
      ClientRequest req;// = processRaw(buffer);
      processRaw(buffer, num_byte,&req);
      //-------------
      string fakename= "google.com";
      struct hostent *hp = gethostbyname(fakename.c_str());
      struct in_addr *ip_addr;
        if (hp == NULL) {
           printf("gethostbyname() failed\n");
        } else {
           printf("%s = ", hp->h_name);
           if ( hp -> h_addr_list[0] != NULL) {
               ip_addr = ( struct in_addr*)( hp -> h_addr_list[0]);
               char* ip = inet_ntoa( *( struct in_addr*)( hp -> h_addr_list[0]));
               printf("%8x \n", ip_addr->s_addr);
               printf( "%s -",ip );
           }
           printf("\n");
        }

        //-----------------

      printf("Magic: %08x\nTML: %04x \n GID: %2x\nchecksum: %2x\nrequestID: %2x\n",req.magicNumber, req.tml,
                                  req.GID,
                                  req.checksum, req.requestID);
      //display(&req.hostInfo, req.tml-9);

      if (req.error){
        printf("\n >>>> invalid request:...\n");
        InvalidResponse *resp = NULL;
        resp->magicNumber = htonl(0x4a6f7921);
        resp->tml = htons(9);
        resp->GID = 7;
        resp->checksum = 0;
        resp->errorCode = req.error;
        size_t struct_total_length = 9;
        char datagram[struct_total_length] = {0};
        memcpy(datagram, resp, struct_total_length);
        printf("----------------Invalid Response Content (%ld bytes)---------------\n", struct_total_length);
        display(datagram, sizeof(datagram));
        resp->checksum = getChecksum(datagram, sizeof(datagram));
        sendto(this->sock,resp,9,0,(struct sockaddr *)&client,sizeof(client));
        free(resp);
        resp = NULL;
        //add the other info for a invalid request
      }else{
        printf("\n >>>> processing request:...\n");
        ValidResponse *res = NULL;
        string ipstr = resolveHostnames(req.hostList, req.tml-9);
        printf("ok.\n");
        res = (struct ValidResponse *)malloc(sizeof(struct ValidResponse) + ipstr.length());
        res->magicNumber = htonl(0x4a6f7921);
        res->tml = htons(ipstr.length() + 9);
        res->GID = 7;
        res->checksum = 0;
        res->requestID = req.requestID;
        memcpy(&res->ipAddresses, ipstr.c_str(), ipstr.length());
        size_t struct_total_length = sizeof (ValidResponse) + ipstr.length();
        char datagram[struct_total_length] = {0};
	memcpy(datagram, res, struct_total_length);

        printf("----------------Packet Content(%ld bytes) --------------- \n", struct_total_length);
        display(datagram, sizeof(datagram));
        res->checksum = getChecksum(datagram, sizeof(datagram));
        printf("\nsending response: %ld bytes\n",sizeof(res));
        //then send the response message back to sender;
        sendto(this->sock,res,struct_total_length,0,(struct sockaddr *)&client,sizeof(client));
        printf("response sent.\n");
        free(res);
        res = NULL;
      }


    }
  }
  close(this->sock);
}

void ServerUDP::processRaw(char *rawpacket, size_t num_byte, struct ClientRequest* result){
  printf("\n------------- parsing raw data ---------------------\n");
  memcpy(&result->magicNumber, &rawpacket[0], sizeof(result->magicNumber));
  result->magicNumber = ntohl(result->magicNumber);
  memcpy(&result->tml, &rawpacket[4], sizeof(result->tml));
  result->tml = ntohs(result->tml);
  memcpy(&result->GID, &rawpacket[6], sizeof(result->GID));
  memcpy(&result->checksum, &rawpacket[7], sizeof(result->checksum));
  memcpy(&result->requestID, &rawpacket[8], sizeof(result->requestID));
  char hosts_requested[result->tml-9];
  memcpy(hosts_requested, &rawpacket[9], result->tml-9);
  strncpy(result->hostList, hosts_requested, sizeof(hosts_requested));
  result->error = 0b0000;
  if (result->magicNumber != 0x4a6f7921){
      result->error = result->error | 0b0001;
      printf("magic number error \n");
  }
  //if (result.checksum != getChecksum(buffer, int(num_byte)) ){
  //if (result->checksum != result->checksum){
  rawpacket[7] = 0; //set checksum to 0 for calculation
  if(result->checksum != getChecksum(rawpacket, int(num_byte)) ) {
      result->error = result->error | 0b0100;
      printf("checksum error \n");
  }
  if (result->tml != num_byte) {
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
char ServerUDP::getChecksum(char* msg, int num_bytes){
  printf("\n--------- Compute Checksum --------- \n");
  int currentByte = 0;
  while (currentByte < num_bytes){
    printf("%2x: ",  msg[currentByte]);
    currentByte++;
  }
  // do checksum magic
	//assume the checksum is set to 0 prior to this function
	//sum all bytes
	int currentSum = 0;
	for(int i = 0; i < num_bytes; i++){
		currentSum += msg[i];
		//handle carry
		if(currentSum > 255){
			currentSum = currentSum - 256 + 1;
		}
	}
	//print sum
	printf("Sum result: %d", currentSum);

	//bitwise one complement of sum
	unsigned int compSum = (unsigned int) ~currentSum & 0xff;
	char finalSum = (char) compSum;

  printf("\n--------- Compute Checksum --------- \n");
  return finalSum;
  //return 0;
}
//return a byte?string of ip address from a string of hosts
//input might be: "10google.com12facebook.com"
string ServerUDP::resolveHostnames(char* msg, int num_bytes){
  printf("\n---------resolving host names-----------\n");

  return "hostnames";

  //parse the message to get list of host names off
  //call hostent * ip = gethostbyname("google.com");
  int currentByte = 0;
  //struct hostent *he;
  //struct in_addr **addr_list;
  //string ipstr = "";
  while (currentByte < num_bytes){

    int host_size = 0;
    memcpy(&host_size, &msg[currentByte], 1);
    printf("current host_size is: %d: \n", host_size);
    char hostname[host_size+1];
    memcpy(&hostname, &msg[currentByte+1], host_size);
    display(hostname, host_size);
    //if ((he = gethostbyname(msg[currentByte])) == NULL) {
      //printf("Failed to find hostname");
    //}
    //addr_list = (struct in_addr **)he->h_addr_list;
    //ipstr += inet_ntoa(*addr_list[currentByte]);
    currentByte+= host_size-1;
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
