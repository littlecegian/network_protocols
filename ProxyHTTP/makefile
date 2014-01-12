COMPILER=g++
CFLAGS=-Wall
all: http_proxy
http_proxy: server client
server:
	$(COMPILER) $(CFLAGS) server.cpp -o server
client:
	$(COMPILER) $(CFLAGS) client.cpp -o client
clean:
	rm -rf *.o server client
