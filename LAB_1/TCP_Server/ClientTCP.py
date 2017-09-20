#!/usr/bin/python                                                               

from socket import *
import time
import sys
import struct

class ClientTCP:
   #keep track of request number                                                
   requestID = 0

   #initialize client                                                           
   def __init__(self, servername, portnumber):
      self.servername = servername
      self.portnumber = portnumber
      self.tcpSocket = self.connectToServer((servername, portnumber))

   #connect to TCP server                                                       
   def connectToServer(self, servAddr):
      tcpSocket = socket(AF_INET, SOCK_STREAM)
      tcpSocket.connect(servAddr)
      
      return tcpSocket
      
#number of consonants in s
   def cLength(self, s):
		self.sendMessage(5, s)
		resp = self.receiveMessage(1024)
		resTML, resRid, resAns = struct.unpack('!HHH', resp[:6])

		return resTml, resRid, resAns

#remove vowels in s
   def Disemvowel(self, s):
		self.sendMessage(80, s)
		resp = self.receiveMessage(1024)
		resTML, resRid, resAns = struct.unpack('!HHH', resp[:6])

		return resTML, resRid, resAns

#change letters in s to uppercase
   def Uppercasing(self, s):
		self.sendMessage(10, s)
		resp = self.receiveMessage(1024)
		resTML, resRid = struct.unpack('!HHH', resp[:6])

		return resTML, resRid, resAns

#recieve message from server      
   def receiveMessage(self, respLen):
		resp = self.tcpSocket.recv(respLen)
      
		return resp

#send message to server
   def sendMessage(self, operation, s):
		tml = len(s)
		rid = ClientTCP.requestID = ClientTCP.requestID+1
		messageHeader = struct.pack('!HHB',tml,rid,operation)
		message = str(messageHeader) + s
		self.tcpSocket.sendall(message)

#main
#argv: client servername portnumber operation string
if __name__ == "__main__":
   if len(sys.argv) != 5:
      print "usage: client servername portnumber operation string"       
      sys.exit()
   client = sys.argv[0]
   servername = sys.argv[1]
   portnumber = sys.argv[2]
   operation = sys.argv [3]
   s = sys.argv[4]
   portnumber = int(portnumber)
   operation = int(operation)
   client = ClientTCP(servername, portnumber)
   if operation == 5:
      startTime = time.time()
      print "Number of consonants in the string \"{}\".".format(s)
      result = client.cLength(s)
      endTime = time.time()
   elif operation == 80:
      startTime = time.time()
      print "Disemvowel the string \"{}\".".format(s)
      result = client.Disemvowel(s)
      endTime = time.time()
   elif operation == 10:
      startTime = time.time()
      print "Uppercase the string \"{}\".".format(s)
      result = client.Uppercase(s)
      endTime = time.time()
   else:
      print "Invalid Operation"
      sys.exit()
   print "\nRequestId: {}".format(result[1])
   print "\nResponse:  {}".format(result[2])
   print "\nRound trip time: {}s".format(endTime-startTime)
   sys.exit()
