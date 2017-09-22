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
       resp = self.receiveMessage(4096)
       resTML, resRid, resAns = struct.unpack('!B B B', resp[:3])
       return resTML, resRid, resAns

#remove vowels in s
   def Disemvowel(self, s):
       self.sendMessage(80, s)
       resp = self.receiveMessage(4096)
       resTML, resRid = struct.unpack('!B B', resp[:2])
       resAns = str(resp[2:])
       return resTML, resRid, resAns

#change letters in s to uppercase
   def Uppercasing(self, s):
       self.sendMessage(10, s)
       resp = self.receiveMessage(4096)
       resTML, resRid = struct.unpack('!B B', resp[:2])
       resAns = str(resp[2:])
       return resTML, resRid, resAns

#recieve message from server
   def receiveMessage(self, respLen):
       resp = self.tcpSocket.recv(respLen)
       return resp

#send message to server
   def sendMessage(self, operation, s):
       tml = 3 + len(s)
       rid = ClientTCP.requestID 
       ClientTCP.requestID = ClientTCP.requestID+1
       messageHeader = struct.pack('!B B B',tml,rid,operation)
       message = messageHeader + s
       self.tcpSocket.sendall(message)
      
   def run(self):
      while(1):
         s = raw_input("Enter message: ")
         operation = raw_input("Enter operation code: ")
         operation = int(operation)
         if operation == 5:
            startTime = time.time()
            print "Number of consonants in the string \"{}\"".format(s)
            tml, rid, result = client.cLength(s)
            endTime = time.time()
         elif operation == 80:
            startTime = time.time()
            print "Disemvowel the string \"{}\"".format(s)
            tml, rid, result = client.Disemvowel(s)
            endTime = time.time()
         elif operation == 10:
            startTime = time.time()
            print "Uppercase the string \"{}\"".format(s)
            tml, rid, result = client.Uppercasing(s)
            endTime = time.time()
         else:
            print "Invalid Operation"
            continue
         print "\nTML: {}".format(tml)
         print "\nRequestId: {}".format(rid)
         print "\nResponse: {}".format(result)
         print "\nRound trip time: {}s".format(endTime-startTime)

#main
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
      print "Number of consonants in the string \"{}\"".format(s)
      tml, rid, result = self.cLength(s)
      endTime = time.time()
   elif operation == 80:
      startTime = time.time()
      print "Disemvowel the string \"{}\"".format(s)
      tml, rid, result = self.Disemvowel(s)
      endTime = time.time()
   elif operation == 10:
      startTime = time.time()
      print "Uppercase the string \"{}\"".format(s)
      tml, rid, result = self.Uppercasing(s)
      endTime = time.time()
   else:
      print "Invalid Operation"
      sys.exit()
   print "\nTML: {}".format(tml)
   print "\nRequestId: {}".format(rid)
   print "\nResponse: {}".format(result)
   print "\nRound trip time: {}s".format(endTime-startTime)
   client.run()
   sys.exit()
