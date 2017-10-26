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
                ip_bytes  = self.get_response(server_msg)
                if ip_bytes != -1:
                    #parse ip and print it off.
                    print ([hex(ord(c)) for c in ip_bytes])
		    for x in range(0, len(hosts)):
			if x == 0:
			    start = 0
			else:	
			    start = x*4 
			byte1 = int(ord(ip_bytes[start]))
			byte2 = int(ord(ip_bytes[start+1]))
			byte3 = int(ord(ip_bytes[start+2]))
			byte4 = int(ord(ip_bytes[start+3]))
			ip_address = str(byte1) + "." + str(byte2) + "." + str(byte3) + "." + str(byte4)
			print(hosts[x]),
			print(ip_address)
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
        valid_response = False
    	trials = 0
    	while not valid_response and trials < 7:
    	    trials = trials + 1
    	    #print(self.server_addr)
            self.sock.sendto(msg,self.server_addr)
            #print >>sys.stderr, 'waiting for server response'
            data, server = self.sock.recvfrom(4096)
            print("server raw response:")
            self.print_as_hex(data)
            if len(data) == 9 :
                magic, tml, GID, checksum, bec = self.unpack_invalid_response(data)
                print("Validate response: received error code from server. Retransmitting")
            elif len(data) < 9:
                print("Validate response: length too short. Retransmitting")
            elif len(data) > 9:
                print("Validating response ... ")
                magic, tml, GID, checksum, rid, ips = self.unpack_response(data)
		
                # if you calculate checksum on the data including the checksum,
                # you should get 0 -> correct data.
                if int(self.get_checksum(data, tml)) == 0:
                    valid_response = True
                    return ips
                else:
                    print("[checksum error] - computed checksum: ")
                    print(hex(self.get_checksum(data, tml)))
            else:
                print("Validate response: received invalid response. Retransmitting")
        if not valid_response and trials == 7:
    	     print("No  valid response after 7 trials.")
    	     return -1

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
        #print(len(data))
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
            self.print_as_hex(ips)
            return magic, tml, GID, checksum, rid, ips
        else:
            return 0, 0, 0, 0, 0, 0

    def unpack_invalid_response(self, data):
        print(len(data))
        magic, tml, GID, checksum, bec = struct.unpack_from("!LHBBB", data[0:])
        print("-------Invalid Response-------")
        print ("Magic Number:")
        print (hex(magic))
        print ("tml:")
        print(hex(tml))
        print ("GID:")
        print(hex(GID))
        print ("checksum:")
        print(hex(checksum))
        print ("bec:")
        print(hex(bec))
        return magic, tml, GID, checksum, bec

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
        print(tml);
        print ('---- Computing Checksum ----\n')
        #TODO verify checksum is calculated correctly

        print ([hex(ord(c)) for c in msg])
        currentsum = 0;
        array = bytearray(msg)

        print("sum of byte array")
        print(hex(sum(array)))
        for x in range(0, tml):
            currentsum += array[x];
            if currentsum > 255:
                currentsum = currentsum - 256 + 1
        print("returning checksum: ")
        print(hex(~currentsum & 0xff))
        return (~currentsum & 0xff)

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
