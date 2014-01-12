			README




*  Product: TFTP Server
*  Date: Oct 1 2013



------------------------------------------------------------
* 1.  OVERVIEW
------------------------------------------------------------
TFTP(Trivial File Transfer Protocol) is a simple protocol used to transfer files.Each nonterminal packet is acknowledged separately.TFTP is implemented over UDP, which is a connectionless protocol, which means there are very few guarantees of the transmission medium.  


---------------------------------------------------------------
* 2.  SYSTEM REQUIREMENTS 
---------------------------------------------------------------
Linux environment is required. GCC has to be installed on the operating system.



---------------------------------------------------------------
* 3.  CONTENTS OF THE PROJECT
---------------------------------------------------------------
1.server.cpp
2.makefile
3.readme.txt

 
------------------------------------------------------------
* 4.  LIST OF COMMAND LINE FLAG OPTIONS
------------------------------------------------------------

To operate the program successfully,
 
1.Run the makefile first with the command make -f makefile.
2.the server should start on the IP and port supplied on the command line:<./server server_ip server_port>.
3.We can request for mutiple files from different TFTP clients simultaneously.

IN Client side,

1) open terminal and put in tftp

	if you have installed tftp-hpa, 
		it will prompt saying (to). type in the server ip and port number here
	otherwise
		type in "connect serverip port"

2) set octet mode by typing in "mode octet"

3) to receive a file, type in "get filename"


---------------------------------------------------------------
* 5.  ERROR CASES
---------------------------------------------------------------

It is possible that sometimes the server maynot recognize the packet. In that case, connection details of the particular client from which the server got the corrupt packet will be dropped.

If the file size is larger than 30 MB, the server will send back an error

If the filename does not exist, the server will throw an error again

Do not try to WRQ to the server. It is not implemented as of now.



---------------------------------------------------------------
* 6.  FUNCTION OF THE PROGRAM
---------------------------------------------------------------


Server:

The server starts on the IP and port number given on the command line.  After a read request is received, if the file is not found an error message is returned.If the requested file size exceeds 30 MB, an error message is returned. Otherwise, the file is transferred in chunks of 512 bytes. when the last chunk of data is sent, the client acknowledges end of transfer and stops pinging the server

When the file size is exactly a multiple of 512 bytes, the server sends a dummy packet to let the client know the end of transfer

A Map STL is used to store the list of connections. It contains, the file opened, last block number successfully sent, amount of data read in the last iteration. Using these values a server will be able to identify the connection uniquely.





ECEN 602 Project BY 

Ravi Sankar Raju
Xinyi Cai