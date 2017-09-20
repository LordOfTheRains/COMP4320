from socket import *
import sys



C_LENGTH = 1
DISEMVOWEL = 2
UPCASE = 3

class ClientUDP:

    def __init__(self, ip, port):
        self.destination_ip = ip
        self.destination_port = int(port)
        self.sock = socket(AF_INET, SOCK_DGRAM)
        # do initialization here

    def run(self):
        while(1):
            message = raw_input("Enter Message: ")
            ops_code = raw_input("Enter operation code: ")
            try:
                ops = int(ops_code)
                if ops != C_LENGTH and ops != DISEMVOWEL and ops != UPCASE:
                    print ("Invalid Operation Code")
                    #do C_LENGTH
                    break;
                else:
                    server_msg = self.get_packed_message(message, ops_code)
                    print(self.get_response(server_msg))
            except ValueError as ex:
                print ("operation code must be a number")
                break;


    def get_response(self, msg):
        self.sock.sendto(msg,self.destination_ip,self.destination_port)
        return "some rresult from server"

    def get_packed_message(self, msg, ops):
        # packet the message to send to server
        server_msg = ops+msg
        return server_msg




#python ClientUDP.py 127.0.0.0 80

if __name__ == "__main__":
    if len(sys.argv) != 3:#require port number and ip
        print "usage: server_ip portnumber"
        sys.exit()
    server_ip = sys.argv[1];
    port = sys.argv[2];

    udpClient = ClientUDP(server_ip,port)
    udpClient.run()
