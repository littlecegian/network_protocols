#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <iostream>
#include <map>

using namespace std;

struct sbcp_header{
    unsigned int version:9;
    unsigned int type:7; // max value can be 128
    unsigned int length:16;  
};

struct sbcp_attribute {
    unsigned int type:16;
    unsigned int length:16;
    char content[1024];
};

struct sbcp_packet{
    struct sbcp_header header;
    struct sbcp_attribute attribute[3];
};

void construct_packet(struct sbcp_packet *packet_ptr, int version, int header_type, int attribute_type, char *content, int attribute_type2 = 0, char *content2 = NULL) // this is a generic function to construct packets to be sent to clients
{
	packet_ptr->header.version = version;
	packet_ptr->header.type = header_type;
	packet_ptr->attribute[0].type = attribute_type;
	strcpy(packet_ptr->attribute[0].content, content); 
	packet_ptr->attribute[0].length = sizeof(struct sbcp_attribute);
	packet_ptr->attribute[1].type = attribute_type2;
	if(content2 != NULL)
		strcpy(packet_ptr->attribute[1].content, content2);
	packet_ptr->attribute[1].length = sizeof(struct sbcp_attribute); 
	packet_ptr->header.length = sizeof(struct sbcp_packet);
}

bool username_already_registered(char client_names[100][20], int number_of_clients, char username[20]) // sees if the username is unique
{
	if(username == NULL)
		return true;
	for(int i=0; i<= number_of_clients; i++)
	{
		if(strcmp(client_names[i], username) == 0) // client_names is the array where list of all clients are maintained
			return true;
	}
	return false;
}

void get_list_of_active_members(char client_names[100][20], int maxfd, int clientid, char *temp) // to display to the new comer, this function will be called
{
	temp[0] = '\0';
	for(int i = 0; i <= maxfd; i++)
	{
		if(i == clientid)
			continue;
		if(strlen(client_names[i]) > 0)
		{
			strcat(temp, client_names[i]);
			strcat(temp, ", ");
		}
	}
	
	int length = strlen(temp);
	if(length > 0)
		temp[length-2] = '\0';
}

int main(int argc, char *argv[])
{
	if(argc < 4)
	{
		cout << "proper usage is: obj_file server_ip server_port max_clients";
		exit(0);
	}
	struct sbcp_packet packet1, packet2;
	fd_set read_fds, temp_fds;
	int listenfd =0, client_fd = 0, client_limit, header_type, request_buffer_size, max_fd, fd_iterator, i, client_count = 0;
	struct sockaddr_in server_addr, client_addr;
	char request_buffer[1024], temp_buffer[1024];;
	char client_names[100][20];
	socklen_t client_length;
	char int_buffer[1024];
	char *names;

	names = new char[2000];
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&server_addr, '0', sizeof(server_addr));
	bzero((char*)request_buffer, 1024);
	bzero((char *)client_names, 100*20);

	FD_ZERO(&read_fds);
	FD_ZERO(&temp_fds);
	FD_SET(listenfd, &read_fds);

	client_limit = atoi(argv[3]);
	max_fd = listenfd;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[2]));

	int bind_var = bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(bind_var == -1)
	{
		cout << "bind error" << endl;
		exit(0);
	}
	
	int listen_var = listen(listenfd, client_limit); //listen for argv[3] clients at a time
	if(listen_var == -1)
	{
		cout << "unable to listen" << endl;
		exit(0);
	}

	while(1)
	{
	// cout << endl << "new while" << endl;
	temp_fds = read_fds;
	if(select(max_fd+1, &temp_fds, NULL, NULL, 0) == -1)
		{
		exit(1);
		}
	for(fd_iterator=0;fd_iterator <= max_fd; fd_iterator++)
		{
		// cout << "max_fd is " << max_fd << "; monitoring of fd " << fd_iterator <<" - " << FD_ISSET(fd_iterator, &temp_fds) << " readfd value " <<  FD_ISSET(fd_iterator, &read_fds) << endl;
		if(FD_ISSET(fd_iterator, &temp_fds))
			{
			if(fd_iterator == listenfd) // this means that something has changed in the listening socket. probably somebody is trying to connect
				{
				client_length = sizeof(client_addr);
				client_fd = accept(listenfd, (struct sockaddr*)&client_addr, &client_length);
				if(client_fd == -1)
					{
					cout << "Error Connecting the new client";
					}
				else
					{
					FD_SET(client_fd, &read_fds); // add the new client's socket to the list of sockets that has to be monitored.
					max_fd = (client_fd > max_fd)? client_fd : max_fd;
					}
				}
			else
				{
				request_buffer_size = read(fd_iterator, (struct sbcp_packet *)&packet1, sizeof(packet1));
				if(request_buffer_size > 0)
					{
					if(packet1.header.type == 2) // means join
						{
						if(client_count >= client_limit || username_already_registered(client_names, max_fd, packet1.attribute[0].content))  // in this case, the client cannot join the chatroom
						{
							if(client_count < client_limit)
							{
								if(strcmp(client_names[fd_iterator], packet1.attribute[0].content) == 0)
								{
									strcpy(temp_buffer, "You have already Joined the Chat Room. Please continue chatting.");
									header_type = 10; // means that this is a message from server itself to a single client. just display
								}
								else
								{
									strcpy(temp_buffer, "Your username has already been chosen. Please pick another and continue.");
									header_type = 5; // NAK message
								}
							}
							else
							{
								strcpy(temp_buffer, "Chat Room is full. Please Join after sometime.");
								header_type = 5; // NAK
							}
							construct_packet(&packet2, 3, header_type, 1, temp_buffer);
							write(fd_iterator, (void *)&packet2, sizeof(packet2));
						}
						else
						{	strncpy(client_names[fd_iterator], packet1.attribute[0].content, 20);
							client_count++;
							construct_packet(&packet2, 3, 8, 2, client_names[fd_iterator]); // this is the online message sent to other chat clients
							sprintf(int_buffer, "%d", client_count);
							get_list_of_active_members(client_names, max_fd, fd_iterator, names);
							construct_packet(&packet1, 3, 7, 3, int_buffer, 5, names); // this is the ACK message
							for(int i=3; i<=max_fd; i++)
								{
								if(i == listenfd)
									continue;
								if(i == fd_iterator)
									{
									write(i, (void *)&packet1, sizeof(packet2));
									continue;
									}
								if(strlen(client_names[i]) > 0)
									{
									write(i, (void *)&packet2, sizeof(packet2));
									}
								}
							}
						}
					else
						{
						if(strlen(client_names[fd_iterator]) > 0)
							{
							construct_packet(&packet2, 3, 3, 2, client_names[fd_iterator], 4, packet1.attribute[0].content); //this is the FWD message from the server
							for(i=3; i<=max_fd; i++)
								{
								if(i == listenfd || i== fd_iterator)
									continue;
								if(strlen(client_names[i]) > 0)
									{
									write(i, (void *)&packet2, sizeof(packet2));
									}
								}
							}
						else
							{
							cout << "data sent to no particular chat room" << endl; // for server debugging purposes
							}
						}
					}
				else if (request_buffer_size == 0) //means client has shut himself down
					{
					if(strlen(client_names[fd_iterator]) > 0)
						{
						client_count--;
						FD_CLR(fd_iterator, &read_fds);
						construct_packet(&packet2, 3, 6, 2, client_names[fd_iterator]); // OFFLINE message is sent to all the users.
						for(i=3;i<=max_fd;i++)
							{
							if(i == listenfd || i == fd_iterator)
								continue;
							if(strlen(client_names[i]) > 0)
								{
								write(i, (void *)&packet2, sizeof(packet2));
								}
							}
						client_names[fd_iterator][0] = '\0';
						}
					FD_CLR(fd_iterator, &read_fds);
					close(fd_iterator);
					}
				}
			}
		}
	}
	close(client_fd);
}
