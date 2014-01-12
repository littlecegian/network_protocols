COMPILER=g++
CFLAGS=-Wall -g
all: tftp
tftp: server 
server:
	$(COMPILER) $(CFLAGS) server.cpp -o server

clean:
	rm -rf *.o server
