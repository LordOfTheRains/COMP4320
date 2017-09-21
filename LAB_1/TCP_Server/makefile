CC=g++ -std=c++11
INC_DIR = ./
CFLAGS=-c -Wall -I$(INC_DIR)
DEPS = $(DEPS)*.h

all: demo
#	UDPServerDisplay.o ServerUDP.o

demo:  ServerTCP.cpp
	$(CC) ServerTCP.cpp -o demo

ServerTCP.o: ServerTCP.cpp
	$(CC)  ServerTCP.cpp

clean:
	    rm -rf *o all

run_server:
	   ./demo 10017
