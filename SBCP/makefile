COMPILER=g++
CFLAGS=-Wall
all: broadcast
broadcast: server client
server:
	$(COMPILER) $(CFLAGS) server.cpp -o server
client:
	$(COMPILER) $(CFLAGS) client.cpp -o client
clean:
	rm -rf *.o server client
