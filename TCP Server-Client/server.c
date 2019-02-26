#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#define PORT 8080
#define MAX_SIZE 1020000

void * socketThread(void * arg)
{
    /* This code segment is shared by each client thread. */
    int valread;
    char buffer[MAX_SIZE] = {0};
    char *response = "Hello World!\n";
    int new_socket = *((int *)arg);
    // Infinite loop to keep listening to port for data from client.
    while(1){
        for(int i = 0; i < MAX_SIZE; ++i)
            buffer[i] = '\0';
        valread = read( new_socket , buffer, MAX_SIZE);  // read information from client
        
        /* Some processing done to extract first word separated by default delimiter white space. */
        char word[MAX_SIZE] = {0};
        int b = 0;
        while(buffer[b] != ' ' && buffer[b] != '\0')
        {
            word[b] = buffer[b];
            b++;
        }
        word[b] = '\0';
        /*******************************************************************************************/
        
        if(strcmp(buffer, "listall") == 0)
        {
            // Received command listall from client
            char ls[MAX_SIZE];
            int c = 0;
            DIR *dir_name;
            struct dirent *dir; // Special data structure to hold details of files in directories.
            dir_name = opendir("."); // Open current directory
            if(dir_name)
            {
                /* Keeps reading the contents of directory and puts them in ls character array */
                while((dir = readdir(dir_name)) != NULL)
                {
                    for(int j = 0; j < strlen(dir->d_name); ++j)
                        ls[c++] = dir->d_name[j];
                    ls[c++] = '\n';
                }
                ls[c - 1] = '\0';
                closedir(dir_name); // Close directory
            }
            response = &ls[0];
        }
        else if(strcmp(buffer, "exit") == 0){
            /* Special response to client if it wants to disconnect*/
            response = "Thanks for connecting! Have a nice Day!";
            send(new_socket , response , strlen(response) , 0 );  // use sendto() and recvfrom() for DGRAM
            printf("Response sent to client!\n");
            break; // Break out of infinite loop as client wants to disconnect
        }
        else if(strcmp(word, "send") == 0){
            /* This is the case when a user requests a file from the server. */
            char * token = strtok(buffer, " ");
            token = strtok(NULL, " ");
            /* If only one token, user has forgot to send file name */
            if(token == NULL){
                response = "What file do you want from the server??";
            }
            else{
                /* Checks for the existence of the file in the current directory */
                DIR *dir_name;
                struct dirent *dir;
                dir_name = opendir(".");
                int found = 0;
                if(dir_name)
                {
                    while((dir = readdir(dir_name)) != NULL)
                    {
                        if(strcmp(dir->d_name, token) == 0)
                        {
                            found = 1;
                        }
                    }
                    closedir(dir_name);
                }
                /*****************************************************************/
                if(found){
                    // If requested file is a valid file...
                    FILE * file;
                    file = fopen(token, "r"); // Open file
                    char response_data[MAX_SIZE];
                    /* Read the contents of the opened file into character array */
                    for(int i = 0; i < MAX_SIZE; ++i)
                        response_data[i] = '\0';
                    char ch;
                    int r = 0;
                    while((ch = fgetc(file)) != EOF && r < (MAX_SIZE - 1))
                        response_data[r++] = ch;
                    /************************************************************/
                    
                    response = &response_data[0]; // Sets the response variable with value of response_data character array.
                }
                else{
                    // Client has requested an invalid file.
                    response = "File Not found!";
                }
            }
        }
        else{
            // Dealing with invalid commands
            response = "Invalid Command!";
        }
        send(new_socket , response , strlen(response) , 0 );  // use sendto() and recvfrom() for DGRAM
        printf("Response sent to client!\n");
    }
    pthread_exit(NULL); // exits the current thread so that the parent knows.
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket;
    struct sockaddr_in address;  // sockaddr_in - references elements of the socket address. "in" for internet
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc.
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    pthread_t thread[105];
    int cnt = 0;
    while(1){
        // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if(new_socket < 0)
        {
            perror("accept");
            continue;
        }
        // Whenever a new client requests connection, create a thread. The thread starts execution from socketThread routine defined above.
        /* A thread is a light weight process, which shares same code segment, global variables etc but has its own local stack,etc. */
        int ret = pthread_create(&thread[cnt++], NULL, socketThread, &new_socket);
        if(ret != 0)
        {
            perror("Failed to create thread");
            continue;
        }
    }
    return 0;
}
