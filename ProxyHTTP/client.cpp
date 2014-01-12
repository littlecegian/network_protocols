#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>

using namespace std;


//usage: object_file user_name server_ip port
int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cout << "proper usage is: object_file server_ip port url" << endl;
        exit(0);
    }

    int server_connection_fd;
    struct sockaddr_in serv_addr;
    string url, host, document_path, get_request;
    char get_request_buffer[1024];
    url.assign(argv[3]);
    int host_end, host_size, url_size;
    url_size = url.length();
    if(url.substr(0,7) != "http://")
    {
        cout << "Please make sure that the protocol is be http://" << endl;
        exit(0);
    }
    for(host_end = 7; host_end <= url_size && url[host_end] != '/'; host_end++);
    if(url[host_end] == '/')
        host_end--;
    host_size = host_end - 6;
    host.assign(url.substr(7, host_size));
    document_path.assign(url.substr(host_end+1));
    get_request.assign("GET ");
    get_request.append(document_path);
    get_request.append(" HTTP/1.0\r\n");
    get_request.append("HOST: ");
    get_request.append(host);
    get_request.append("\r\nConnection: close\r\nUser-Agent: raviswebcrawler/1.0\r\nAccept-Charset: ISO-8859-1,UTF-8;q=0.7,*;q=0.7\r\nCache-Control: no-cache\r\nAccept-Language: de,en;q=0.7,en-us;q=0.3\r\nReferer: none\r\n\r\n");
    cout << "get request is" << endl << get_request << endl << endl;
    strcpy(get_request_buffer, get_request.c_str());
    server_connection_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_connection_fd < 0) 
        cout << "Error opening socket" << endl;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    if (connect(server_connection_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        cout << "Error connecting" << endl;
    else
    {   
        write(server_connection_fd, (void *)&get_request_buffer, strlen(get_request_buffer));
        int read_size = read(server_connection_fd, get_request_buffer, sizeof(get_request_buffer));
        get_request_buffer[read_size] = '\0';
        cout << "content from server is " << endl << endl << get_request_buffer;
    }
    close(server_connection_fd);
    return 0;
}