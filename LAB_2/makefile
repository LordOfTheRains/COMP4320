CC=g++ -g -std=c++11
INC_DIR = ./
CFLAGS=-c -Wall -I$(INC_DIR)
DEPS = $(DEPS)*.h

all: demo

#	UDPServerDisplay.o ServerUDP.o

demo: UDPServerDisplay.o ServerUDP.o
	$(CC) UDPServerDisplay.o ServerUDP.o -o demo

UDPServerDisplay.o:  UDPServerDisplay.cpp ServerUDP.h
	$(CC) $(CFLAGS) UDPServerDisplay.cpp

ServerUDP.o: ServerUDP.cpp ServerUDP.h
	$(CC) $(CFLAGS) ServerUDP.cpp

clean:
	    rm -rf *o all

run_server:
		make
	   ./demo 10017

run_client:
	python ClientUDP.py 0.0.0.0 10017 120 google.com facebook.com jinx.com
