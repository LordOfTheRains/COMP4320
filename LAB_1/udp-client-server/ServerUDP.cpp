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
      req.msgLength = int(buffer[0]);
      req.requestID = int(buffer[1]);
      req.operation = int(buffer[2]);
      string content(&buffer[3], &buffer[3] + req.msgLength-3);
      req.message = content;
      req.error = 0;
      if (req.error){
        printf("\n >>>> invalid request:...\n");
      }else{
        Response resp = getResponse(&req);
        printf("\nsending response: %llu\n",resp.result);
        printf("response size: %lu\n",sizeof(resp));
        //then send the response message back to sender;
        sendto(this->sock,&resp,sizeof(resp),0,(struct sockaddr *)&client,sizeof(client));
        char respchar[sizeof(Response)];
        memcpy(respchar, &resp, sizeof(Response));
        display(respchar,(int)sizeof(resp));
        printf("response sent.\n");
      }
      /* Got something, just send it back */

    }
  }
  close(this->sock);
}

ServerUDP::ClientRequest ServerUDP::processRaw(char *msg){
  ClientRequest req;
  req.msgLength = int(msg[0]);
  req.requestID = int(msg[1]);
  req.operation = int(msg[2]);
  string content(&msg[3], &msg[3] + req.msgLength);
  req.message = content;
  req.error = 0;
  return req;
}


// returns the final message ready to be sent back to client
ServerUDP::Response ServerUDP::getResponse(ClientRequest *req){
  //int reqID = req->requestID;
  string msg = req->message;
  string response;
  printf("Message is [%s]\n",msg.c_str());
  printf("Operation is [%d]\n", req->operation);
  switch(req->operation) {
    case 0x05: //getCLength
      puts("Counting number of consonants...");
      response = getCLength(msg);
      break;
    case 0x50: //disemvoweling
      puts("Getting rid of vowels...");
      response = disemvoweling(msg);
      break;
    case 0x80:// upperCasing
      puts("Capitalizing everything...");
      response = upperCasing(msg);
      break;
    default:
      puts("Operation not supported...");
      response = "error operation code." + req->operation;
      break;
  }
  //pack the result message
  Response res;
  res.tml = response.size() + 2;
  res.requestID = req->requestID;
  res.result = toBinary(response);
  return res;

}

//returns the number of consonants in msg
//getCLength("Hello") == "3"
string ServerUDP::getCLength(string msg){
  int result = 0;
  char vowels[] = {'a', 'e', 'i', 'o', 'u', 'y'};
  bool isVowel = false;
  // iterate message and see if the letter is vowel
  // if not vowel, increment result

  for(char& c : msg) {//get each letter
    if (isalpha(c)) {
      cout << "One character: " << c << "\n";
      for (char& v: vowels){//iterate  vowel list
        //cout << "One vowel: " << v << "\n";
        if (tolower(c) == v){//letter is a consonant
          isVowel = true;
        }
      }
      if (!isVowel){
        result++;
        puts("------was consonant;");
      }
      isVowel = false;
    }
  }
  printf("getCLength is [%d]\n", result);
  return to_string(result);
}


//return message without vowels
//disemvoweling("Hello") == "HLL"
string ServerUDP::disemvoweling(string msg){
  char vowels[] = {'a', 'e', 'i', 'o', 'u', 'y'};
  string result = "";
  bool isVowel = false;
  for(char& c : msg) {//get each letter
    if (isalpha(c)) {
      //cout << "One character: " << c << "\n";
      for (char& v: vowels){//iterate  vowel list
        //cout << "One vowel: " << v << "\n";
        if (tolower(c) == v){//letter is a consonant
          isVowel = true;
        }
      }
      if (!isVowel){
        result += c;
        puts("------was consonant------");
      }
      isVowel = false;
    }else{
        result += c;
    }
  }

  printf("disemvoweling is [%s]\n", result.c_str());
  return result;
}


//return message uppercasing every letter
// upperCasing("Hello") == "HELLO"
string ServerUDP::upperCasing(string msg){
  //string upped[msg.size()];
  for (auto & let: msg) let = toupper(let);
  puts(msg.c_str());
  return msg;
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
