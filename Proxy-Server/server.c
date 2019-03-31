#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#define PORT 8081
#define MAX_SIZE 1020000

void * socketThread(void * arg)
{
    int valread;
    char buffer[MAX_SIZE];
    char *response = "Response from destination server!\n";
    int new_socket = *((int *)arg);
    for(int i = 0; i < MAX_SIZE; ++i)
            buffer[i] = '\0';
    valread = read( new_socket , buffer, MAX_SIZE); 
    send(new_socket , response , strlen(response) , 0 );
    close(new_socket);
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    /*********************** Setting up the server **************************/
    /**************** Code copy pasted from Geek for Geeks ******************/
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
        pthread_t thread;
        int ret = pthread_create(&thread, NULL, socketThread, &new_socket);
        if(ret != 0)
        {
            perror("Failed to create thread");
            continue;
        }
    }
    return 0;
}
