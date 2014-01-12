#include <iostream>
#include <string>
#include <list>
#include <stdlib.h>
#include <fstream>
#include <functional>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctime>
#include <sstream>
#include <map>
#include <time.h>

#define LIST_SIZE 10
using namespace std;

int get_document_path_length(string);
int get_host_length(string, int);
bool invalid_http_version(string get_request, int start);

struct CacheEntry
{
	string url;
	string html_response;
	struct tm last_modified_time;
	struct tm expiry_time;
};

map<string, struct CacheEntry> cache_mapping;
list<struct CacheEntry> cache;
list<struct CacheEntry>::iterator list_iterator;

bool is_entry_present_in_cache(string key)
{
	return (cache_mapping.find(key) != cache_mapping.end());
}

void print_list(){
	cout << endl << "-------------------------------------------------" << endl;
    cout << "Cache List - MRU to LRU" << endl;
    cout << "-------------------------------------------------" << endl;
    int list = 1;
    for(list_iterator = cache.begin(); list_iterator != cache.end(); list_iterator++)
    {
	    struct CacheEntry temp = *list_iterator;
	    cout << list++ << ". " << temp.url << endl;
    }
    cout << endl << endl;
}

int timecmp(struct tm time_1, struct tm time_2)
{
	return(difftime(mktime(&time_1), mktime(&time_2)));
}

struct tm get_last_modified_date_from_response(string data)
{
	struct tm last_modified_time;
	int start = data.find("Last-Modified:") + 15, i;
	for(i=start; data[i]!='\r'; i++);
	// cout << "date is " << data.substr(start, i-start) << endl;
	strptime(data.substr(start+5, i-start-4).c_str(), "%d %b %Y %H:%M:%S", &last_modified_time);
													// Wed, 02 Oct 2013 22:12:39 GMT
	return last_modified_time;
}

struct tm get_expiry_date_from_response(string data)
{
	struct tm expiry_time;
	int start = data.find("Expires:") + 9, i;
	for(i=start; data[i]!='\r'; i++);
	// cout << "expires is " << data.substr(start, i-start) << endl;
	strptime(data.substr(start+5, i-start-4).c_str(), "%d %b %Y %H:%M:%S", &expiry_time);
	// cout << expiry_time.tm_hour << endl;
	return expiry_time;
}

string validate_and_obtain_entry_from_cache(string key, string conditional_get_request, string host)
{
	string data;
	time_t rawtime;
	struct tm *time_now;
	struct CacheEntry popped_entry;
	cout << "there's a hit in the cache for the url  -  " << key << ". checking for validity" << endl;
	//there's a hit in the cache. validate and handle it. assume for now that the cache block is always valid
	// print_list();
	for (list_iterator = cache.begin(); list_iterator != cache.end(); ++list_iterator) 
	{
		popped_entry = *list_iterator;
    	if(popped_entry.url == key)
    		break;
	}
	cache.erase(list_iterator);
	time(&rawtime);
  	time_now = gmtime(&rawtime);
	// popped_entry.expiry_time
	cout << endl << "expiry time of the cache entry is " << asctime(&popped_entry.expiry_time);
	cout << "time now is " << asctime(time_now) << endl;
	if(timecmp(popped_entry.expiry_time, *time_now) <= 0)
	{
		// the cache entry has expired. use head to get the necessary details. construction of head request
		cout << "cache entry has expired. sending a HEAD command to see if the page has been modified since." << endl;
		int webserverfd = 0, byte_written, response_size;
		struct sockaddr_in webserver_addr;
		struct hostent *server;
		char head_request_buffer[1024], html_response[4096];
		struct tm latest_last_modified_time;
		string html_response_string;
		// cout << conditional_get_request;
		conditional_get_request.replace(0, 3, "HEAD");
		// cout << conditional_get_request;
		strcpy(head_request_buffer, conditional_get_request.c_str());
		webserverfd = socket(AF_INET, SOCK_STREAM, 0);
		webserver_addr.sin_family = AF_INET;
		server = gethostbyname(host.c_str());
		memcpy(&webserver_addr.sin_addr, server->h_addr_list[0], server->h_length);
		// webserver_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		webserver_addr.sin_port = htons(80);
		if (connect(webserverfd, (struct sockaddr *) &webserver_addr, sizeof(webserver_addr)) < 0) 
		    cout << "Error connecting to webserver" << endl;
		else
		{
			byte_written = write(webserverfd, (void *)&head_request_buffer, strlen(head_request_buffer));
			if(byte_written == -1)
				cout << "Error while forwarding the HEAD request" << endl;
			response_size = read(webserverfd, html_response, sizeof(html_response));
			if(response_size == -1)
				cout << "Error while forwarding the HEAD request" << endl;
			// cout << "response size from webserver for HEAD is " << response_size << endl;
			// cout << "response from webserver for HEAD is " << html_response << endl;
			html_response_string.assign(html_response);
			latest_last_modified_time = get_last_modified_date_from_response(html_response_string);
			cout << endl << "old last modified time from the cache entry is " << asctime(&popped_entry.last_modified_time);
			cout << "new last modified time from the HEAD command is " << asctime(&latest_last_modified_time) << endl;
			if(timecmp(latest_last_modified_time, popped_entry.last_modified_time) > 0)
			{
				// invalid entry. fetch from server again.
				cout << "The page has been modified since it was last fetched into the cache. Issuing a GET to update the page in cache" << endl;
				// cout << conditional_get_request;
				conditional_get_request.replace(0, 4, "GET");
				// cout << conditional_get_request;
				char get_request_buffer[1024];
				strcpy(get_request_buffer, conditional_get_request.c_str());
				webserverfd = socket(AF_INET, SOCK_STREAM, 0);
				webserver_addr.sin_family = AF_INET;
				server = gethostbyname(host.c_str());
				memcpy(&webserver_addr.sin_addr, server->h_addr_list[0], server->h_length);
				// webserver_addr.sin_addr.s_addr = htonl(INADDR_ANY);
				webserver_addr.sin_port = htons(80);
				if (connect(webserverfd, (struct sockaddr *) &webserver_addr, sizeof(webserver_addr)) < 0) 
				    cout << "Error connecting to webserver" << endl;
				else
				{
					byte_written = write(webserverfd, (void *)&get_request_buffer, strlen(get_request_buffer));
					if(byte_written == -1)
						cout << "Error while forwarding the GET after HEAD request" << endl;
					// cout << "bytes written to web server " << byte_written << endl;
					response_size = read(webserverfd, html_response, sizeof(html_response));
					if(response_size == -1)
						cout << "Error while forwarding the GET after HEAD request" << endl;
					// update_cache_with_new_entry(url, html_response);
					// cout << "response size from webserver for GET after HEAD is " << response_size << endl;
					// cout << "response from webserver for GET after HEAD is " << html_response << endl;
					popped_entry.html_response.assign(html_response);
					popped_entry.expiry_time = get_expiry_date_from_response(html_response);
					popped_entry.last_modified_time = get_last_modified_date_from_response(html_response);
					cout << "cache successfully updated with latest copy." << endl;
				}
			}
			else
			{
				//nothing to do. update expiry time and move on.
				// cout << "old expiry_time is "<< asctime(&popped_entry.expiry_time) << endl;
				cout << "Looks like the page has not been modified. updating the expiry time in the cache and sending the page back to client." << endl;
				cout << "Saved Bandwidth by avoiding an unnecessary GET." << endl;
				popped_entry.expiry_time = get_expiry_date_from_response(html_response_string);
				// cout << "new expiry_time is "<< asctime(&popped_entry.expiry_time) << endl;
			}
		}
		// cout << "Response from web server is " << endl << html_response;
	}
	else
	{
		cout << "The page in the cache has not expired yet. Serving from the cache." << endl;
	}
	cache.push_front(popped_entry);
	print_list();
	return popped_entry.html_response;
}

void update_cache_with_new_entry(string key, string data)
{
	// below stmt used for testing outside vpn. 
	// data.assign("HTTP/1.1 200 OK\r\nLast-Modified: Wed, 02 Oct 2013 22:12:39 GMT\r\nExpires: Fri, 01 Nov 2012 22:12:39 GMT\r\n\r\n");
	string old_key;
	struct CacheEntry entry, temp;
	entry.url.assign(key);
	entry.html_response.assign(data);
	entry.expiry_time = get_expiry_date_from_response(data);
	entry.last_modified_time = get_last_modified_date_from_response(data);
	if(cache.size() < LIST_SIZE)
	{
		//straightforward. create a new cache entry and insert at the front.
		cache_mapping[key] = entry;
		cache.push_front(entry);
	}
	else if (cache.size() == LIST_SIZE)
	{
		//LRU has to be done
		old_key = cache.back().url;
		cache_mapping.erase(cache_mapping.find(old_key));
		cache.pop_back();
		cache_mapping[key] = entry;
		cache.push_front(entry);
	}
	else
	{
		cout << "absurd case. cannot happen" << endl;
	}
	print_list();
	cout << "New entry has been inserted at the front of the cache. expiry time of the new entry is " << asctime(&(entry.expiry_time));
}

int main(int argc, char *argv[])
{
	map<string, struct CacheEntry>::iterator map_pointer;
	if(argc < 3)
	{
		cout << "proper usage is: obj_file server_ip server_port";
		exit(0);
	}
	fd_set read_fds, temp_fds;
	string url, host, document_path, get_request, response_buffer;
	int document_path_size, host_start_location, host_length, byte_written, response_size;
	int webserverfd = 0, listenfd =0, client_fd = 0, request_buffer_size, max_fd, fd_iterator;
	struct sockaddr_in server_addr, client_addr, webserver_addr;
	socklen_t client_length;
	struct hostent *server;
	char get_request_buffer[1024], html_response[4096];

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&server_addr, '0', sizeof(server_addr));

	FD_ZERO(&read_fds);
	FD_ZERO(&temp_fds);
	FD_SET(listenfd, &read_fds);

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
	
	int listen_var = listen(listenfd, 100); //listen for argv[3] clients at a time
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
					// cout << "new client connected. max fd is " << max_fd << endl;
					}
				}
			else
				{
					request_buffer_size = read(fd_iterator, get_request_buffer, sizeof(get_request_buffer));
					get_request_buffer[request_buffer_size] = '\0';
					// cout << request_buffer_size << endl;
					// cout << get_request_buffer << endl;
					get_request.assign(get_request_buffer);
					// cout << get_request;
					if(get_request.substr(0, 3) != "GET")
					{
						cout << "Invalid packet format. request should be GET" << endl;
						close(fd_iterator);
						break;
					}
					else
					{
						//finding document path
						document_path_size = get_document_path_length(get_request);
						document_path = get_request.substr(4, document_path_size);
						// cout << "document_path is " << document_path << endl;

						if(invalid_http_version(get_request, 4+document_path_size+1))
						{
							cout << "HTTP version should be 1.0. Rejecting the Packet" << endl;
							close(fd_iterator);
							break;
						}
						else
						{
							host_start_location = get_request.find("HOST:") + 6;
							host_length = get_host_length(get_request, host_start_location);
							host.assign(get_request.substr(host_start_location, host_length));
							cout << endl <<  "NEW SERVICE INITIATED" << endl << "=========================================================================================================================" << endl;
							cout << "HOST - " << host << endl;
							cout << "DOCUMENT PATH - " << document_path << endl;
							url = host + document_path;
							if(is_entry_present_in_cache(url))
							{
								//entry is already there. if it is stale, repull. or serve the response after doing a conditional get. for now, serve always. put it in the front
								string response_buffer = validate_and_obtain_entry_from_cache(url, get_request, host);
								strcpy(html_response, response_buffer.c_str());
								// cout << "Response from the cache is " << endl << html_response;
							}
							else
							{
								//there's a miss in the cache
								cout << "There's a miss in the cache. Fetching from webserver." << endl << endl;
								webserverfd = socket(AF_INET, SOCK_STREAM, 0);
								webserver_addr.sin_family = AF_INET;
								server = gethostbyname(host.c_str());
								memcpy(&webserver_addr.sin_addr, server->h_addr_list[0], server->h_length);
								// webserver_addr.sin_addr.s_addr = htonl(INADDR_ANY);
								webserver_addr.sin_port = htons(80);
								if (connect(webserverfd, (struct sockaddr *) &webserver_addr, sizeof(webserver_addr)) < 0) 
	    						    cout << "Error connecting to webserver" << endl;
	    						else
								{
									byte_written = write(webserverfd, (void *)&get_request_buffer, strlen(get_request_buffer));
									if(byte_written == -1)
										cout << "Error while forwarding the GET request" << endl;
									// cout << "bytes written to web server " << byte_written << endl;
									response_size = read(webserverfd, html_response, sizeof(html_response));
									response_buffer.assign(html_response);
									if(response_size == -1)
										cout << "Error while forwarding the GET request" << endl;
									if (response_buffer.find("200") != std::string::npos)
										update_cache_with_new_entry(url, html_response);
									else
										strcpy(html_response, "Invalid URL. Please check the Resource Locator\r\n");
									// cout << "response size from webserver is " << response_size << endl;
									// cout << "response from webserver is " << html_response << endl;
								}
								// cout << "Response from web server is " << endl << html_response;
							}
							write(fd_iterator, (void *)&html_response, strlen(html_response));
							memset(&html_response, '\0', sizeof(html_response));
							cout << endl << "Response forwarded to client. END OF SERVICE for this request" << endl << endl;
							cout << "=========================================================================================================================" << endl << endl;
							close(webserverfd);
						}
					}
					FD_CLR(fd_iterator, &read_fds);
					close(fd_iterator);
				}
			}
		}
	}
	close(listenfd);
}



int get_document_path_length(string get_request)
{
	int i = 0;
	for(i=4; get_request[i] != ' '; i++);
	return i-4;
}

bool invalid_http_version(string get_request, int start)
{
	// cout << get_request.substr(start, 8) << endl;
	// cout << (get_request.substr(start, 8) == "HTTP/1.0") << endl; 
	return (get_request.substr(start, 8) != "HTTP/1.0");
}

int get_host_length(string get_request, int start)
{
	int i;
	for(i=start; get_request[i] != '\r'; i++);
	return (i-start);
}