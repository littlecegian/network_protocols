#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

using namespace std;

struct sbcp_header{
    unsigned int version:9;
    unsigned int type:7;
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

//usage: object_file user_name server_ip port
int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cout << "proper usage is: object_file user_name server_ip port" << endl;
        exit(0);
    }

    struct sbcp_packet packet1, packet2;
    packet1.header.version = 3;

    int server_connection_fd, max_fd, fd_iterator;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char user_name[20];
    strcpy(user_name, argv[1]);
    fd_set read_fds, temp_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&temp_fds);
    FD_SET(0, &read_fds);

    char user_input[1024], content_from_server[1024];
    server_connection_fd = socket(AF_INET, SOCK_STREAM, 0);
    FD_SET(server_connection_fd, &read_fds);
    max_fd = server_connection_fd;

    if (server_connection_fd < 0) 
        printf("Error opening socket");
    server = gethostbyname(argv[2]);
    if (server == NULL) {
        printf("Error - no such host\n");
        exit(0);
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(atoi(argv[3]));
    if (connect(server_connection_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        printf("Error connecting");
    else
        cout << "please enter join to connect to the only chat room" << endl;
    
    while(1)
    {
        temp_fds = read_fds;
        if(select(max_fd+1, &temp_fds, NULL, NULL, 0) == -1)
            {
            cout << "monitoring cin and server connection failed for some reason" << endl;
            }
        
        for(fd_iterator=0; fd_iterator<=max_fd; fd_iterator++)
        {
            if (FD_ISSET(fd_iterator, &temp_fds))
            {
                if(fd_iterator == 0) //this is reading from console
                {
                    int read_length = read(0, (char *)&user_input, 1024); // equivalent of reading from socket 0
                    user_input[read_length-1] = (char)NULL; // removing the \n\r from the read input
                    if (strcmp(user_input, "join") == 0)
                    {
                        packet1.header.type = 2; //denotes join command
                        packet1.attribute[0].type = 2; //denotes that you are sending the username as an attribute
                        strcpy(packet1.attribute[0].content, user_name);
                        packet1.attribute[0].length = sizeof(struct sbcp_attribute);
                    }
                    else // this means you are sending the actual chat message
                    {
                        packet1.header.type = 4; //indicates chat
                        packet1.attribute[0].type = 4; //indicates that the content is a chat text
                        strcpy(packet1.attribute[0].content, user_input);
                        packet1.attribute[0].length = sizeof(struct sbcp_attribute);
    
                    }
                    packet1.header.length = sizeof(struct sbcp_packet);
                    write(server_connection_fd, (void *)&packet1, sizeof(packet1));
                }
                else
                {
                    bzero((char *)content_from_server, sizeof(content_from_server));
                    read(fd_iterator, (struct sbcp_packet *)&packet2, sizeof(packet2));
                    switch(packet2.header.type){
                        case 3: //message from another client
                            cout << packet2.attribute[0].content << " says " << packet2.attribute[1].content << endl;
                            break;
                        case 7: //ack from server
                            {
                                int number_of_users = atoi(packet2.attribute[0].content);
                                cout << "Number of Clients in the chat session - " << number_of_users;
                                if(number_of_users == 1)
                                    cout << ". You are the first user in the chatroom. Please wait." << endl;
                                else
                                    cout << ". Other users in the chatroom are " << packet2.attribute[1].content << "." << endl;
                                break;
                            }
                        case 5: //nak from server
                            cout << packet2.attribute[0].content << endl;
                            break;
                        case 6: //offline message
                            cout << packet2.attribute[0].content << " is offline now." << endl;
                            break;
                        case 8: //online message
                            cout << packet2.attribute[0].content << " is online now." << endl;
                            break;
                    }
                }   
            }
        }
    }
    close(server_connection_fd);
    return 0;
}