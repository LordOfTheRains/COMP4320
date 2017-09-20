CC=g++ -std=c++11
INC_DIR = ./
CFLAGS=-c -Wall -I$(INC_DIR)

all: UDPServerDisplay.o ServerUDP.o

%.o: %.cpp $(DEPS)
	    $(CC) -o $@ $< $(CFLAGS)

clean:
	    rm -rf *o all
