from socket import *
import sys
import time
import struct

class ClientUDP:

    def __init__(self, ip, port):
        try:
            self.server_addr = (ip, int(port))
            self.sock = socket(AF_INET, SOCK_DGRAM)
        except ValueError:
            print("invalid port number")
        # do initialization here
    def execute(self, req_id, hosts):
        try:
            req_id = int(req_id)
            if req_id > -1 and req_id < 128:
                server_msg = self.get_packed_message(req_id, hosts)
                server_response  = self.get_response(server_msg)
                #server_response  = "fakenews"
                magic, tml, GID, checksum, rid, ips = self.unpack_response(server_response)
            else:
                print ("Request ID must be between [0-255]")
                return

        except ValueError as ex:
            print ("Request ID must be a number")
            print ex
            return

    def run(self):
        while(1):
            req_id = raw_input("Enter Request ID: ")
            hosts = raw_input("Enter list of host name seperated by space: ")
            host_list = hosts.split(" ")
            self.execute(req_id, host_list)


    def get_response(self, msg):

        #print(self.server_addr)
        self.sock.sendto(msg,self.server_addr)
        #print >>sys.stderr, 'waiting for server response'
        data, server = self.sock.recvfrom(4096)
        print("server raw response")
        self.print_as_hex(data)

        magic, tml, GID, checksum, rid, ips = self.unpack_response(data)
        print("server packed response")
        self.print_as_hex(data)

        return data

    '''
    server response format: for valid request
    0x4a6f7921: 4 bytes
    Tml: 2 bytes
    group id: 1 byte
    checksum: 1 byte
    request id: 1 byte
    ip_address(es): 4 bytes/host
    -------------------------------------------
    server response format: for invalid request
    0x4a6f7921: 4 bytes
    Tml: 2 bytes
    group id: 1 byte
    checksum: 1 byte
    byte error code: 1 byte
    '''

    def unpack_response(self, data):
        print(len(data))
        if len(data) > 9:
            magic, tml, GID, checksum, rid= struct.unpack_from("!LHBBB", data[0:])
            response_size = tml - 9
            unpack_f = "!LHBBB" + str(response_size)+ "s"
            print (unpack_f)
            magic, tml, GID, checksum, rid, ips= struct.unpack_from(unpack_f, data[0:])
            print ("Magic Number:")
            print (hex(magic))
            print ("tml:")
            print(hex(tml))
            print ("GID:")
            print(hex(GID))
            print ("checksum:")
            print(hex(checksum))
            print ("rid:")
            print(hex(rid))
            print ("ips:")
            print (ips)
            return magic, tml, GID, checksum, rid, ips
        return 0, 0, 0, 0, 0, 0

    '''
    request format: for valid request
    0x4a6f7921: 4 bytes
    Tml: 2 bytes
    group id: 1 byte
    checksum: 1 byte
    request id: 1 byte
    **following field is is a pair and there can be a list of these pairs
    ** in a single request
    hostname length: 1 byte
    hostname(s): variable bytes/host
    '''

    def get_packed_message(self, req_id, hosts):
        # packet the message to send to server
        magic = 0x4a6f7921
        host_list_size = 0
        hosts_packed = ""
        host_info_size = 0;
        for host in hosts:
            # pack size in network order
            host_length = len(host.encode('utf-8'))
            length_byte = struct.pack("!B",host_length)
            host_frame = length_byte + host
            host_info_size = host_info_size + host_length + 1
            # pack the host name
            hosts_packed = hosts_packed + host_frame
            self.print_as_hex(hosts_packed)

        print ('---- tml check ----\n')
        tml = host_info_size + 9
        GID = 7
        checksum = 0 # set to zero to compute

        header = struct.pack("!IHBBB",magic, tml, GID, checksum, req_id)
	server_msg = header + hosts_packed

	checksum = self.get_checksum(server_msg, tml) #compute checksum then repack

	header = struct.pack("!IHBBB",magic, tml, GID, checksum, req_id)
	server_msg = header + hosts_packed

        print ('---- server message ----\n')
        self.print_as_hex(server_msg)
        return server_msg

    def get_checksum(self, msg, tml):
        #TODO verify checksum is calculated correctly
        currentsum = 0;
        array = bytearray(msg)
        for x in range(0, tml):
            currentsum += int(array[x]);
            if currentsum > 255:
                currentsum = currentsum - 256 + 1

	checksum = ~currentsum & 0xff
	return checksum

    def print_as_hex (self, msg_string):
        print ":".join("{:02x}".format(ord(c)) for c in msg_string)


#python ClientUDP.py 127.0.0.0 80

if __name__ == "__main__":
    if len(sys.argv) < 5 and len(sys.argv) != 3:
       print "usage: ClientUDP.py [server ip] [server port] [Request ID] [Host name list]..."
       sys.exit()
    server_ip = sys.argv[1]
    port = sys.argv[2]
    udpClient = ClientUDP(server_ip,port)
    if len(sys.argv) == 3:
        udpClient.run()
    if len(sys.argv) > 4:
        req_id = sys.argv[3]
        hosts = []
        num_host = len(sys.argv)
        for i in range(4,num_host):
            host = sys.argv[i]
            print host
            hosts.append(host)
        udpClient.execute(req_id, hosts)
