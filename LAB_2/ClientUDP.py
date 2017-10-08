from socket import *
import sys
import time
import struct

class ClientUDP:

    def __init__(self, ip, port):
        self.request_id = 0
        try:
            self.server_addr = (ip, int(port))
            self.sock = socket(AF_INET, SOCK_DGRAM)
        except ValueError:
            print("invalid port number")
        # do initialization here
    def execute(self, ops, msg):
        try:
            if ops != 5 and ops != 80 and ops != 10:
                print ("Invalid Operation Code")
                print("Operations: [1] - C length; [2] - Disemvowel; [3] - Upcasing")
                break;
            else:
                server_msg = self.get_packed_message(message, ops)
                start_time = time.time()
                tml, rid, response = self.get_response(server_msg)
                end_time = time.time()
                print "\ntml:  {}".format(tml)
                print "\nRequest ID: {}".format(rid)
                print "\nResponse:  {}".format(response)
                print "\nRound trip time: {}s".format(end_time-start_time)
        except ValueError as ex:
            print ("operation code must be a number")
            print ex
            break;

    def run(self):
        while(1):
            message = raw_input("Enter Message: ")
            print("Operations: [1] - C length; [2] - Disemvowel; [3] - Upcasing")
            ops_code = raw_input("Enter operation code: ")
            self.execute(ops, message)


    def get_response(self, msg):

        #print(self.server_addr)
        self.sock.sendto(msg,self.server_addr)
        #print >>sys.stderr, 'waiting for server response'
        data, server = self.sock.recvfrom(4096)
        for i in data:
            print hex(ord(i))
        #print >>sys.stderr, 'received "%b"' % data
        tml = struct.unpack("B", data[0:1])[0]
        rid = struct.unpack("B", data[1:2])[0]
        response = data[2:tml]
        #response =
        print ('---\n')
        for i in response: print hex(ord(i))
        return tml, rid, response

    def get_packed_message(self, msg, ops):
        # packet the message to send to server
        msg_size_byte = len(msg.encode('utf-8'))
        tml = msg_size_byte + 3
        ops_code = 0
        if ops == 5:
            ops_code = 0x05
        elif ops == 80: #disemvoweling
            ops_code = 0x50
        else:#upcasing
            ops_code = 0x0a
        header = struct.pack("!BBB",tml,self.request_id,ops_code)
        server_msg = header + msg
        print repr(server_msg)
        self.request_id = self.request_id+1
        return server_msg




#python ClientUDP.py 127.0.0.0 80

if __name__ == "__main__":
    if len(sys.argv) != 5 or 2:
       print "usage: ClientUDP.py [server ip] [server port] [function code] [string]"
       sys.exit()
    server_ip = sys.argv[1]
    port = sys.argv[2]
    udpClient = ClientUDP(server_ip,port)
    if len(sys.argv) == 2:
        udpClient.run()
    if len(sys.argv) == 5:
        ops = sys.argv[3]
        msg = sys.argv[4]
        udpClient.execute(ops, msg)
