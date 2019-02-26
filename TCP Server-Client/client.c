// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#define PORT 8080
#define MAX_SIZE 1020000
int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr; // Special data structure to hold address of server. 
    char *hello = "Hello from client";
    char buffer[MAX_SIZE] = {0};
    /* Creating a socket for the client to communicate through.*/
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
                                                // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET; // Refers to addresses from internet, IP addresses specifically.
    serv_addr.sin_port = htons(PORT); // Specifies the port through which it is going to connect to server. htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Converts an IP address in numbers-and-dots notation into either a
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    printf("Welcome to the client!\n");
    // Runs an infinite loop to keep taking commands from the user, until exit is entered.
    while(1){
        printf(">>");
        char buffer[MAX_SIZE];
        char store[MAX_SIZE];
        for(int i = 0; i < MAX_SIZE; ++i)
            buffer[i] = '\0';
        /* Accepting input from the user. */
        int len = 0;
        char c;
        scanf("%c", &c);
        buffer[len++] = c;
        while(c != '\n')
        {
            scanf("%c", &c);
            buffer[len++] = c;
        }
        buffer[len - 1] = '\0';
        for(int i = 0; i < MAX_SIZE; ++i)
            store[i] = '\0';
        for(int i = 0; i < len; ++i)
            store[i] = buffer[i];
        /**********************************/
        
        send(sock , buffer , strlen(buffer) , 0 );  // send the message.
        
        for(int i = 0; i < MAX_SIZE; ++i)
            buffer[i] = '\0';
        
        valread = read( sock , buffer, MAX_SIZE);  // receive message back from server, into the buffer
        
        /* Processing according to message from server*/
        if(strcmp(buffer, "Thanks for connecting! Have a nice Day!") == 0){
            /* Exit Client */
            printf("%s\n",buffer);    
            break;
        }
        if(strcmp(buffer, "What file do you want from the server??") == 0){
            /* Handles the case where user enters send but forgets to enter what file he wants */
            printf("%s\n", buffer);
            continue;
        }
        if(strcmp(buffer, "File Not found!") == 0){
            /* Handles the case where user requests a missing file. */
            printf("%s\n", buffer);
            continue;
        }
        char * token = strtok(store, " "); /* Used for tokenizing. token now contains first token of store. Tokens are separated by space character */
        if(strcmp(token, "send") == 0){
            token = strtok(NULL, " "); /* Get next token i.e second token of store */
            FILE *f = fopen(token, "w"); // Opens a file with the name as the one requested for.
            fprintf(f, "%s", buffer); // Write the response from the server to this file.
            fclose(f); // Close file buffer
            printf("File downloaded successfully!\n");
            continue;
        }
        // Corresponds to listall command output
        printf("%s\n", buffer);
    }
    return 0;
}
