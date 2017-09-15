#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdlib>           // For atoi()
#include <stdexcept>

#include <server-udp.h>

using namespace std;

int CLENGTH = 1;
int DISEMVOWEL = 2;
int UPCASE = 3;

ServerUDP::ServerUDP(int port){
  if ((this->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
  	perror("cannot create socket");
  }
  this->port = port;
}

int ServerUDP::configure(){
  struct sockaddr_in myaddr;

  //setup socket and bind
    return 0
}

void ServerUDP::run(){
// run the server continuously.
//not sure if i need to fork for udp if new connection detected.
  while (1){ 	// _M2
    printf("\n >>>> listener: waiting for a datagram...\n");

    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
           (struct sockaddr *)&their_addr, &addr_len)) == -1) {
      perror("recvfrom");
      exit(1);
    }

    printf("listener: got packet from %s\n",
     inet_ntop(their_addr.ss_family,
         get_in_addr((struct sockaddr *)&their_addr),
         s, sizeof s));
    printf("listener: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("listener: packet contains \"%s\"\n", buf);
    display(buf,numbytes); // _M3

    //
    ClientRequest req = processRaw("the packet");
    if (req->error){
      printf("\n >>>> invalid request:...\n");
    }else{
      string resp = getResponse(req);
      //then send the response message back to sender;
    }
  } // _M2
  close(this->sock);

}

ClientRequest ServerUDP::processRaw(string msg){
  ClientRequest req;
  req->msgLength = 10;
  req->requestID = 4;
  req->operation = 1;
  req->message = "messge part";
  req->error = 0;
  return req;
}


// returns the final message ready to be sent back to client
string ServerUDP::getResponse(ClientRequest *req){
  int reqID = req->requestID;
  int ops = req->operation;
  int msg = req->message;
  string response;
  switch(ops) {
    case CLENGTH:
      response = getCLength(msg);
      break;
    case DISEMVOWEL:
      response = disemvoweling(msg);
      break;
    case UPCASE:
      response = upperCasing(msg);
      break;
    default:
      response = "error operation code."
      break;
  }
  //pack the result message
  return "answer";

}

//returns the number of consonants in msg
//getCLength("Hello") == "3"
string ServerUDP::getCLength(string: msg){
  return "1";
}


//return message without vowels
//disemvoweling("Hello") == "HLL"
string ServerUDP::disemvoweling(string:msg){
  return "no vowel";
}


//return message uppercasing every letter
// upperCasing("Hello") == "HELLO"
string ServerUDP::upperCasing(string:msg){
  return "BOOM";
}

void ServerUDP::display(char *Buffer, int length){
  int currentByte, column;

  currentByte = 0;
  printf("\n>>>>>>>>>>>> Content in hexadecimal <<<<<<<<<<<\n");
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
}
