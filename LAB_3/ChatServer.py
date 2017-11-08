from socket import *
import sys
import time
import struct


class ChatServer:

    # creates the udp port
    def __init__(self, ip, port):
        try:
            self.server_addr = (ip, int(port))
            self.sock = socket(AF_INET, SOCK_DGRAM)
            self.sock.bind(self.server_addr)
            self.client_waiting = False
        except ValueError:
            print("invalid port number")
        # do initialization here

    def run(self):
        print 'Chat server listening for request...'
        while(1):
            data, server = self.sock.recvfrom(4096)
            print "Message: ", data
            error = validate_request(data)
            if error == 0:
                client_ip, client_port = self.unpack_request(data)
                if self.client_waiting:
                    response = self.get_connect_response()
                else:
                    self.register(client_ip, client_port)
                    response = self.get_registered_response()
            else:
                response = self.get_invalid_response(error)
            self.sock.sendto(response,self.response)

    # returns 0 if request is valid
    # return error code if invalid
    def validate_request(self,data):
        return True


    #unpacks the response to get ip and port info
    # TODO incomplete implementation
    def unpack_request(self, data):
        return "some ip", "some port"

    def register(self,waiting_ip, waiting_port):
        self.client_waiting = True
        self.waiting_client = (waiting_ip, int(waiting_port))

    #packs the response to return to client indicating
    # TODO incomplete implementation
    def get_registered_response(self, client_port):
        # just packs the waiting client info and return it back to the reqeusting
        # client.
        self.client_waiting = False
        magic = 0x4a6f7921
        GID = 7
        header = struct.pack("!IB",magic,GID)
        response = header + "port number of client here"
        print ('---- registered response message ----\n')
        self.print_as_hex(response)
        return response

    #packs the response to return to client receiving the waiting client info
    # TODO incomplete implementation
    def get_connect_response(self):
        # just packs the waiting client info and return it back to the reqeusting
        # client.
        self.client_waiting = False
        magic = 0x4a6f7921
        GID = 7
        header = struct.pack("!IB",magic,GID)
        response = header + "waitingclient info"
        print ('---- response message ----\n')
        self.print_as_hex(response)
        return response

    def get_invalid_response(self, error_code):
        return "some error message"


#python ChatServer.py 127.0.0.0 80

if __name__ == "__main__":
    if len(sys.argv) != 3:
       print "usage: ChatServer.py [server ip] [server port]"
       sys.exit()
    server_ip = sys.argv[1]
    port = sys.argv[2]
    chat_server = ChatServer(server_ip,port)
    chat_server.run()
