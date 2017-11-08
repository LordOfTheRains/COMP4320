CC=gcc -g
INC_DIR = ./
CFLAGS=-c -Wall -I$(INC_DIR)
DEPS = $(DEPS)*.h

all: ChattyChat

#	UDPServerDisplay.o ServerUDP.o

ChattyChat: ChattyChat.o ChatClient.o
	$(CC) ChattyChat.o ChatClient.o -o ChattyChat

ChattyChat.o:  ChattyChat.c ChatClient.h
	$(CC) $(CFLAGS) ChattyChat.c

ChatClient.o: ChatClient.c ChatClient.h
	$(CC) $(CFLAGS) ChatClient.c

clean:
	    rm -rf *o all

run_client:
		make
	   ./ChattyChat 0.0.0.0 10017 10001

run_server:
	python ChatServer.py 10017
