			README




*  Project - Simple Chat Service Program
*  Date	-  Sep 17 2013


In this project we have simulated a chat room wherein clients can join, chat to other clients, see the list of other clients. When a new client joins, all others will be notified. Also, when a new user joins, he will be notified of the current users in the chatroom. Different types of messages has been controlled with version bits and attributes. 

;-------------------------------------------------------------------------------------------------------------
;		HOW TO RUN
;-------------------------------------------------------------------------------------------------------------

	* Go to the folder and run make
	* After the compilation, run the server as <./server 127.0.0.1 3454 20>. This will host a server in the local address at port 3454 and allows a max of 20 clients in the chatroom
	* Run the first client now as <./client first_one 127.0.0.1 3454>. Basically, the client provides his username along with the ip and port of the server to which he wants to connect
	* Connect few more clients.
	* Type in join from any client to connect to the chatroom. Unless you do this, you will not have joined the chatroom.
	* you can start typing to chat with your buddies now. To quit the chatroom, just press ctrl + c.


;--------------------------------------------------------------------------------------------------------------
;	OTHER TRIVIA
;-------------------------------------------------------------------------------------------------------------

There is a validation on the client name and the number of permissible connections in the chat room. when you violate those, you will not be allowed to join the chat room. 

You need a UNIX system with g++ installed in it.

Files present are
	* server.cpp - server file
	* client.cpp - client file
	* README - this file
	* makefile	- to compile the code


;-------------------------------------------------------------------------------------------------------------
;	ARCHITECTURE
;-------------------------------------------------------------------------------------------------------------

	There is one server running and multiple clients tryint to connect. There can be any number of connection to the server, but the connection to the chat room is limited. A chat room will have unique usernames. users with same name will not be able to connect. However, if a client disconnects, his name and slot will be available for the taking.

	And the clients will be able to see other client's list of names while joining

	when a client joins / leaves, all users in the chat room will be notified. 

	IDLE feature has not been implemented.

	clients can chat as long as they want to. They can sent only text messages of size 1024. thats the limit of the application.
