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
#define PORT 8080
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
    return;
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

    
    /********************************* Send Response to client ******************************/
    send(ptr->socket , &http_request[0] , http_request.size() , 0);
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
