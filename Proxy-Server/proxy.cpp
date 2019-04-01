#include "bits/stdc++.h"
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 20100
#define MAX_SIZE 1024
using namespace std;

struct element{
    int socket;
    string destination_ip;
    string client_ip;
    int destination_port;
    int client_port;
    string method;
    string filename;
};

void parse(string http_request, element * ptr){
    string dest_ip;
    string dest_port;
    string filenm;
    string get_post;

    int i;
    for(i =0;http_request[i]!=' ';i++){
    	get_post.push_back(http_request[i]);
    }
    	
    i += 8;
	for(;http_request[i]!=':';i++)
		dest_ip.push_back(http_request[i]);
	i++;
	for(;http_request[i]!='/';i++)
		dest_port.push_back(http_request[i]);
	i++;
	for(;http_request[i]!=' ';i++)
		filenm.push_back(http_request[i]);

	ptr -> destination_port = atoi(&dest_port[0]);
	ptr -> destination_ip = dest_ip;
	ptr -> filename = filenm;	
	ptr -> method = get_post;
    return;
}

int connect(element * ptr)
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(ptr -> destination_port);
    if(inet_pton(AF_INET, &ptr->destination_ip[0], &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    return sock;
}

string communication(int sock, element * ptr, string message){
    //string message = "Hello from client";
    string request = "";
    char * split = strtok(&message[0], " ");
    int cnt = 0;
    while(split != NULL){
        if(cnt == 0)
            request = split;
        else if(cnt == 1)
            request = request + " /" + ptr->filename;
        else
            request = request + " " + split;
        split = strtok(NULL, " ");
        cnt++;
    }
    send(sock , &request[0] , request.size(), 0);
    char buffer[MAX_SIZE];
    for(int i = 0; i < MAX_SIZE; ++i)
        buffer[i] = '\0';
    read( sock , buffer, MAX_SIZE);
    string response = "";
    while(buffer[0] != '\0'){
        for(int i = 0; i < strlen(buffer); ++i)
            response = response + buffer[i];
        for(int i = 0; i < MAX_SIZE; ++i)
            buffer[i] = '\0';
        read( sock , buffer, MAX_SIZE);    
    }
    return response;
}

void * serveRequest(void * arg)
{
    int valread;
    element * ptr = (element *)arg;
    /********************************** Get HTTP Request ***********************************/
    char buffer[MAX_SIZE] = {0};
    for(int i = 0; i < MAX_SIZE; ++i)
            buffer[i] = '\0';
    valread = read( ptr->socket , buffer, MAX_SIZE);
    string http_request = "";
    for(int i = 0; i < strlen(buffer); ++i)
        http_request = http_request + buffer[i];
    /****************************************************************************************/

    /********************************* Parse HTTP request ***********************************/
    parse(http_request, ptr);
    /****************************************************************************************/
    /********************************* Connect to Server ************************************/
    int server_socket = connect(ptr);
    /****************************************************************************************/
    /************************** Send request to server and get response *********************/
    string response = communication(server_socket, ptr, http_request);
    /****************************************************************************************/
    
    /********************************* Send Response to client ******************************/
    send(ptr->socket , &response[0] , response.size() , 0);
    /****************************************************************************************/
    
    /********************************* Close connection *************************************/
    close(ptr->socket);
    pthread_exit(NULL);
    /****************************************************************************************/
}

int main(int argc, char const *argv[])
{
    /************************ Setting up the proxy server ***********************************/
    /************************ Code copy pasted from Geek for Geeks **************************/
    int server_fd, new_socket;
    struct sockaddr_in address; 
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    /****** Link : https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/ ******/
    while(1){
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if(new_socket < 0)
        {
            perror("accept");
            continue;
        }

        element e;
        e.client_port = ntohs(address.sin_port);
        e.client_ip = inet_ntoa(address.sin_addr);
        e.socket = new_socket;
        pthread_t thread;
        int ret = pthread_create(&thread, NULL, serveRequest, &e);
        if(ret != 0)
        {
            perror("Failed to create thread");
            continue;
        }
    }
    return 0;
}
