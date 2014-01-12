README




*  Product: Simple HTTP Proxy
*  Date: Oct 15 2013



*  CONTENTS OF THIS DOCUMENT
------------------------------------------------------------
This document contains the following sections:

1.  Overview
2.  System Requirements
3.  Contents of the Files
4.  List of Available Command Line Flag Options 
5.  Functions of the program



------------------------------------------------------------
* 1.  OVERVIEW
------------------------------------------------------------
The Hypertext Transfer Protocol (HTTP) is an application-level protocol with the lightness and speed necessary for distributed, collaborative, hypermedia information systems.It is a generic, stateless, object-oriented protocol which can be used for many tasks, such as name servers and distributed object management systems, through extension of its request methods (commands).
A feature of HTTP is the typing of data representation, allowing systems to be built independently of the data being transferred.



---------------------------------------------------------------
* 2.  SYSTEM REQUIREMENTS 
---------------------------------------------------------------
Linux environment is required. GCC has to be installed on the operating system.



---------------------------------------------------------------
* 3.  CONTENTS OF THE FILE
---------------------------------------------------------------
1.server.cpp
2.client.cpp
3.makefile
4.readme.txt

 
------------------------------------------------------------
* 4.  LIST OF COMMAND LINE FLAG OPTIONS
------------------------------------------------------------

To operate the program successfully,
 
1.Run the makefile first with the command make -f makefile.
2.the proxy should start on the IP and port supplied on the command line:./server <ip to bind> <port to bind> 
3.We can request information by: ./client <proxy address> <proxy port> <url to retrieve>



---------------------------------------------------------------
* 5.  FUNCTION OF THE PROGRAM
---------------------------------------------------------------

After the client input the command line, the proxy server receives a request. It will first check its  cached data. If the data is available, it forwards it to the client. and also prints the cache content in its console. it is not checked if the document is expired, as of now. if the content is not in the cache, it is fetched from the webserver by creating a connection to the host specified in the get request. it then also updates its own cache copy and moves on. If the cache is already full, the least recently used block is kicked out. the block that is in the rear end of the list is the least recently used one. For every hit that happens, the block is moved to the front of the queue. and hence the one that was not used for a long time will be in the end. 

The LRU process is clearly shown in the cache contents with 1. being the MRU and 10. being the LRU

implementation of conditional get and HEAD has to be done yet.


ECEN 602 Project by, 

Ravi Sankar Raju
Xinyi Cai

