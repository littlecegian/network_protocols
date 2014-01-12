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
#include <utility>

using namespace std;

// defining all the basic structures here
// this is the structure which stores information unique to a data connection. Note that same client can have multiple connections
struct tftp_connection_info{
	FILE *file;
	int recent_block_number;
	int recently_sent_chunk_size;
};
// you can see all the structure that the protocol uses at the bottom of the file

char* create_packet(char * data, int opcode, int code) // code param can be either block_code / error_code creates a data/error packet
{
	char *data_to_be_sent = (char *)malloc(516); // for both error and data packet size is same
	*((unsigned short *) data_to_be_sent) = htons(opcode);
	char *error_code_pointer = data_to_be_sent + 2;
	*((unsigned short *)error_code_pointer) = htons(code);
	memcpy(data_to_be_sent + 4, data, 512); //copying the actual data
	return data_to_be_sent;
}

int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		cout << "proper usage is: obj_file server_ip server_port";
		exit(0);
	}

	map<int, struct tftp_connection_info> tftp_connections;   // STL used to store different connections. so that fragmentation is possible
	map<int, struct tftp_connection_info>::iterator map_pointer;	// to iterate throught the STL. needed when cleanup happens
	struct tftp_connection_info temp_info_packet;
	fd_set read_fds, temp_fds;
	int control_fd = 0, data_fd = 0, opcode, optval = 1, file_length, request_buffer_size, response_buffer_size, max_fd, fd_iterator;
	struct sockaddr_in server_addr, client_addr, temp_data_bind_addr;
	socklen_t client_length;
	client_length = sizeof(client_addr);
	char *request_buffer = (char *)malloc(516), *response_buffer = (char *)malloc(516), *data = (char *)malloc(512), *filename;

	if( (control_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		cout << "error in creating the dgram socket. Please check" << endl;
	}
	memset(&server_addr, '0', sizeof(server_addr));

	FD_ZERO(&read_fds);
	FD_ZERO(&temp_fds);
	FD_SET(control_fd, &read_fds); // initializing the monitor list with RRQ port

	max_fd = control_fd;
	inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
	server_addr.sin_family = AF_INET;	server_addr.sin_port = htons(atoi(argv[2]));
	memset(&(server_addr.sin_zero),'\0', 8);

	int bind_var = bind(control_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)); // no need to listen / accept. just bind. udp
	if(bind_var == -1)
	{
		cout << "bind error" << endl;
		exit(0);
	}
	if(setsockopt(control_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
	{
		cout << "error while trying to set sockopt option" << endl;
		exit(1);
	}


	while(1)
	{
		temp_fds = read_fds;
		if(select(max_fd+1, &temp_fds, NULL, NULL, 0) == -1)
		{
			exit(1);
		}
		for(fd_iterator=0;fd_iterator <= max_fd; fd_iterator++)
		{
			if(FD_ISSET(fd_iterator, &temp_fds)) // some activity has happened
			{
				if((request_buffer_size = recvfrom(fd_iterator, request_buffer, 516, 0, (struct sockaddr *)&client_addr, &client_length)) <= 0)
				{
					cout << "error in receiving from client" << endl;
					exit(1);
				}
				else
				{
					opcode = ntohs(*((unsigned short *) request_buffer));
					filename = &request_buffer[2];
					switch(opcode) // RRQ from a client
					{
						case 1:
							{
								if ((data_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
								{
									cout << "error while creating socket for data transfer" << endl;
									exit(1);
								}
								temp_data_bind_addr.sin_family = AF_INET;
								temp_data_bind_addr.sin_port = htons(0);
								inet_pton(AF_INET, argv[1], &temp_data_bind_addr.sin_addr);
								memset(&temp_data_bind_addr.sin_zero, '\0', 8);

								if(bind(data_fd, (struct sockaddr *)&temp_data_bind_addr, sizeof(struct sockaddr)) == -1)
								{
									cout << "unable to bind to" << data_fd << endl;
									exit(1);
								}
								if(setsockopt(data_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
								{
									cout << "error while trying to set sockopt option" << endl;
									exit(1);
								}
								FD_SET(data_fd, &read_fds); // add to master set
								if (data_fd > max_fd) // keep track of the maximum
								{
									max_fd = data_fd;
								}
								
								FILE *file = fopen(filename, "rb");
								if (file == NULL)
								{
									memset(response_buffer, '\0', 516);
									response_buffer = create_packet((char *)"File Not Found", 5, 1);
									if((response_buffer_size = sendto(data_fd, response_buffer, 516, 0, (struct sockaddr *)&client_addr, client_length)) <= 0) // send the initial 512 bytes + 4 bytes of header info
									{
										cout << "error while sending error packet" << endl;
										exit(1);
									}
									else
									{
										close(data_fd);
										FD_CLR(data_fd, &read_fds);
									}
								}
								else
								{
									fseek(file, 0, SEEK_END); // this and the next line will tell us the length of the file
									file_length = ftell(file);	// says where the pointer currently is
									if(file_length > 31457280)
									{

										memset(response_buffer, '\0', 516);
										response_buffer = create_packet((char *)"File Size Too Large", 5, 3);	// string has to be typecasted
										if((response_buffer_size = sendto(data_fd, response_buffer, 516, 0, (struct sockaddr *)&client_addr, client_length)) <= 0)
										{
											cout << "error while sending error packet" << endl;
											exit(1);
										}
										else
										{
											fclose(file);  // always termination should be removing client from connection list, closing file and socket and removing it from the master list
											close(data_fd);
											FD_CLR(data_fd, &read_fds);
										}
									}
									else
									{ // this is where we are actually sending the data - for the first time
										rewind(file);	// this is how you go to the beggining of the file. 
										temp_info_packet.file = file;	// setting up the map values
										temp_info_packet.recent_block_number = 1;
										temp_info_packet.recently_sent_chunk_size = 512;
										tftp_connections[data_fd] = temp_info_packet; // initialize the map
										tftp_connections[data_fd].recently_sent_chunk_size = fread(data, 1, 512, file);
										memset(response_buffer, '\0', 516);
										response_buffer = create_packet(data, 3, 1);
										if((response_buffer_size = sendto(data_fd, response_buffer, tftp_connections[data_fd].recently_sent_chunk_size + 4, 0, (struct sockaddr *)&client_addr, client_length)) <= 0)
										{
											cout << "error while sending error packet" << endl;
											exit(1);
										}
									}
								}
							}
							break;
						case 4:
							{
								if (tftp_connections[fd_iterator].recently_sent_chunk_size < 512) // this means that the data transfer has been completed from the server. and in most cases, its a duplicate ack
								{
									fclose(tftp_connections[fd_iterator].file);
									FD_CLR(fd_iterator, &read_fds);
									close(fd_iterator);
									map_pointer = tftp_connections.find(fd_iterator);
									tftp_connections.erase(map_pointer);
								}
								else
								{
									memset(data, '\0', 512);
									tftp_connections[fd_iterator].recently_sent_chunk_size = fread(data, 1, tftp_connections[fd_iterator].recently_sent_chunk_size, tftp_connections[fd_iterator].file); // if the file size = 512 * n, then this will read 0 bytes in the (n+1)th block. this would indicate the server that the data transfer has been completed.
									tftp_connections[fd_iterator].recent_block_number +=1;
									memset(response_buffer, '\0', 516);
									response_buffer = create_packet(data, 3, tftp_connections[fd_iterator].recent_block_number);
									response_buffer_size = sendto(fd_iterator, response_buffer, tftp_connections[fd_iterator].recently_sent_chunk_size + 4, 0, (struct sockaddr *)&client_addr, client_length);
									
									if( response_buffer_size <= 0)
									{
										cout << "error while sending error packet" << endl;
										exit(1);
									}
									memset(data, '\0', 512);
									memset(response_buffer, '\0', 516);
									memset(request_buffer, '\0', 516);
								}
							}
							break;
						default:
							{
								//time to cleanup. the client has gone rougue
								memset(response_buffer,'\0',516);
								memset(request_buffer,'\0',516);
								memset(data, '\0', 516);
								cout << "Unrecognizable packet received. Dropping it."; //ideally, this will never happen as the client will not send junk
								printf("Socket %d has gone rogue\n", fd_iterator);
								fclose(tftp_connections[fd_iterator].file);
								FD_CLR(fd_iterator, &read_fds);
								close(fd_iterator);
								map_pointer = tftp_connections.find(fd_iterator);
								tftp_connections.erase(map_pointer);
							}
							break;
					}
				}
			}
		}
	}
return 0;
}



/*

struct read_request_packet{
	uint16_t opcode;
	char *filename;
	char *mode;
}__attribute__((packed));

struct ack_packet{
	uint16_t opcode;
	uint16_t block_number;
}__attribute__((packed));

struct data_packet
{
	uint16_t opcode;
	uint16_t block_number;
	char *data;
}__attribute__((packed));

struct error_packet
{
	uint16_t opcode;
	uint16_t error_code;
	char *error_message;
}__attribute__((packed));

These packet structures are followed when data is written into / read out of a buffer
*/