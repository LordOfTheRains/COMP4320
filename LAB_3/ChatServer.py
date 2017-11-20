from socket import *
import sys
import time
import struct


class ChatServer:

    # creates the udp port
    def __init__(self,port):
        try:
            self.server_addr = ('0.0.0.0',int(port))
            self.sock = socket(AF_INET, SOCK_DGRAM)
            self.sock.bind(self.server_addr)
            self.client_waiting = False
        except ValueError:
            print("invalid port number")
        # do initialization here

    def run(self):
        print 'Chat server listening for request...'
        while(1):
            data, client = self.sock.recvfrom(4096)
            client_ip = self.ip2int(client[0])
            error, client_port, GID = self.unpack_request(data)
            if error == 0:
                if self.client_waiting:
                    response = self.get_connect_response()
                    print("Flushing waiting client: " + client[0] + ":" + str(client_port))
                    self.waiting_ip = 0
                    self.waiting_port = 0
                    self.client_waiting = False
                else:
                    self.waiting_ip = client_ip
                    self.waiting_port = client_port
                    self.client_waiting = True
                    print("registered client: " + client[0] + ":" + str(client_port))
                    response = self.get_registered_response(client_port)
            else:
                response = self.get_invalid_response(error)
            self.sock.sendto(response,client)

    #unpacks the response to get ip and port info
    # TODO incomplete implementation
    def unpack_request(self, data):
        error = 0x00;
        if len(data) == 7:
            magic,port, GID= struct.unpack_from("!LHB", data[0:])
            if magic != 0x4a6f7921:
                error = error | 1
            if self.portOutOfRange(port):
                error = error | 4
            print ("Magic Number:" + hex(magic))
            print ("port:" + hex(port))
            print ("GID:" + hex(GID))
            return error, port, GID
        else:
            error = error| 2
            print ("data size not 7" + str(len(data)))
            return error, 0, 0

    def portOutOfRange(self, portnum):
        min_range = 10010+5*7
        max_range = min_range + 4
        return portnum > max_range or portnum < min_range



    #packs the response to return to client indicating
    # TODO incomplete implementation
    def get_registered_response(self, client_port):
        # just packs the waiting client info and return it back to the reqeusting
        # client.
        magic = 0x4a6f7921
        GID = 7
        response = struct.pack("!IBH",magic,GID, client_port)
        print ('---- registered response message ----\n')
        self.print_as_hex(response)
        return response

    #packs the response to return to client receiving the waiting client info
    # TODO incomplete implementation
    def get_connect_response(self):
        # just packs the waiting client info and return it back to the reqeusting
        # client.
        magic = 0x4a6f7921
        GID = 7
        response = struct.pack("!IIHB",magic,self.waiting_ip, self.waiting_port,GID)
        print ('---- waiting client info response message ----\n')
        self.print_as_hex(response)
        return response

    def get_invalid_response(self, error_code):
        magic = 0x4a6f7921
        GID = 7
        response = struct.pack("!IBBB",magic,GID, 0, error_code)
        print ('---- error response message ----\n')
        self.print_as_hex(response)
        return response

    def print_as_hex (self, msg_string):
        print ":".join("{:02x}".format(ord(c)) for c in msg_string)

    def ip2int(self, addr):
        return struct.unpack("!I", inet_aton(addr))[0]
#python ChatServer.py 127.0.0.0 80

if __name__ == "__main__":
    if len(sys.argv) != 2:
       print "usage: ChatServer.py [server port]"
       sys.exit()
    port = sys.argv[1]
    chat_server = ChatServer(port)
    chat_server.run()
