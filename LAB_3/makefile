CC=gcc -g -std=c99
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

run_client_a:
		make
	   ./ChattyChat 0.0.0.0 10017 10045

run_client_b:
	   		make
	   	   ./ChattyChat 0.0.0.0 10017 10046

run_server:
	python ChatServer.py 10017
