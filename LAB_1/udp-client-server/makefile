CC=g++ -std=c++11
INC_DIR = ./
CFLAGS=-c -Wall -I$(INC_DIR)

all: UDPServerDisplay.o ServerUDP.o

%.o: %.cpp $(DEPS)
	    $(CC) -o $@ $< $(CFLAGS)

clean:
	    rm -rf *o all

run_server:
	   ./USDisplay 10017

run_client:
		python ClientUDP.py 0.0.0.0 10017
