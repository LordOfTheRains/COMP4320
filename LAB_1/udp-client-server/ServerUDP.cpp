#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdlib>           // For atoi()
#include <stdexcept>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include <ServerUDP.h>

using namespace std;



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
// run the server continuously.
//not sure if i need to fork for udp if new connection detected.
	int numbytes;
  int rv;
  int MAXBUFLEN = 100;
  int client_sock;
	char buf[MAXBUFLEN];
	struct sockaddr_storage their_addr;
  struct addrinfo hints, *clients, *client;
	socklen_t addr_len;
  while (0){ 	// _M2
    addr_len = sizeof their_addr;

    if ((rv = getaddrinfo(NULL, to_string(this->port).c_str(), &hints, &clients)) != 0) {
		  fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		  return;
	  }
    // loop through all the results and bind to the first for now,
    // should implement new thread or process for multiple active socket bind
    for(client = clients;client != NULL;
      client = client->ai_next) {

  		if ((client_sock = socket(client->ai_family, client->ai_socktype,
  				client->ai_protocol)) == -1) {
          perror("listener: socket");
          continue;
		  }
      //bind to client to get data
  		if (bind(client_sock, client->ai_addr, client->ai_addrlen) == -1) {
  			close(client_sock);
  			perror("listener: bind");
  			continue;
  		}
		    break;
	  }
    // store data received ffrom client socket in buf and print it
    if ((numbytes = recvfrom(this->sock, buf, MAXBUFLEN-1 , 0,
           (struct sockaddr *)&their_addr, &addr_len)) == -1) {
      perror("recvfrom");
      exit(1);
    }
    printf("listener: packet contains \"%s\"\n", buf);
    display(buf,numbytes);
    // handle this packet
    ClientRequest req = processRaw(buf);
    if (req.error){
      printf("\n >>>> invalid request:...\n");
    }else{
      string resp = getResponse(&req);
      //then send the response message back to sender;
    }
  }
  close(this->sock);
    /*
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
*/
}

ServerUDP::ClientRequest ServerUDP::processRaw(string msg){
  ClientRequest req;
  req.msgLength = 10;
  req.requestID = 4;
  req.operation = 1;
  req.message = "messge part";
  req.error = 0;
  return req;
}


// returns the final message ready to be sent back to client
string ServerUDP::getResponse(ClientRequest *req){
  //int reqID = req->requestID;
  int ops = req->operation;
  string msg = req->message;
  string response;
  switch(ops) {
    case 1: //getCLength
      response = getCLength(msg);
      break;
    case 2: //disemvoweling
      response = disemvoweling(msg);
      break;
    case 3:// upperCasing
      response = upperCasing(msg);
      break;
    default:
      response = "error operation code.";
      break;
  }
  //pack the result message
  return "answer";

}

//returns the number of consonants in msg
//getCLength("Hello") == "3"
string ServerUDP::getCLength(string msg){
  int result = 0;
  char vowels[] = {'a', 'e', 'i', 'o', 'u'};
  // iterate message and see if the letter is vowel
  // if not vowel, increment result

  for(auto c : msg) {//get each letter
    for (auto v: vowels){//iterate  vowel list
      if (c != v){//letter is a consonant
        result ++;
      }
    }
  }
  return to_string(result);
}


//return message without vowels
//disemvoweling("Hello") == "HLL"
string ServerUDP::disemvoweling(string msg){
  char vowels[] = {'a', 'e', 'i', 'o', 'u'};
  string result = "";
  for (auto c : msg) {
    for (auto v: vowels){//iterate  vowel list
          if (c != v){//letter is a consonant
            result += c;//add letter to result
          }
    }
  }
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
}
