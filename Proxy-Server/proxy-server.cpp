#include "bits/stdc++.h"
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>
#include <stdlib.h>
#include <string>

#define PORT 20100
#define MAX_SIZE 1024
using namespace std;
static const size_t npos = -1;
struct element{
    int socket;
    string destination_ip;
    string client_ip;
    int destination_port;
    int client_port;
    bool isAuthenticated;
    string method;
    string date_time;
    string filename;
};


map <string, vector<time_t> > cache_time;
map <string , int> cache_loc;
int idx;
/**************************base64 encode and decode functions*************************************/
/* 
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}
std::string base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}
/******************************************************************************************/
bool authenticate(string S){
    bool success = false;
    ifstream file("username_password.txt");
    if(file.is_open()){
        string line;
        while(getline(file, line)){
            if(line == S){
                success = true;
            }
        }
        file.close();
    }
    return success;
}
/******************************************************************************************/
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
    string auth = "";
    int idx = http_request.find("Authorization: Basic ") + 21;
    for(int i = idx; http_request[i] != '\n'; ++i)
        auth += http_request[i];
    ptr->isAuthenticated = authenticate(base64_decode(auth));
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
/************************************************************************************************/
string cacheControl(int sock, element * ptr, string message){
    string request = "";
    char * split = strtok(&message[0], " \n");
    int cnt = 0;
    while(split != NULL){
        if(cnt == 0)
            request += split;
        else if(cnt == 1)
            request = request + " /" + ptr->filename;
        else
            request = request + " " + split;
        split = strtok(NULL, " ");
        cnt++;
    }
    request = request.substr(0, request.find("Host:")) + "If-Modified-Since: " + ptr->date_time + "\n" + request.substr(request.find("Host:"));
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
bool isModified(string response){
  if(response.find("HTTP/1.0 304 Not Modified") ==  std::string::npos)
    return true;
  return false;
}
/********************************************************************************************/
bool isBlackList(string ip, int port){
  bool success = false;
  ifstream file("./proxy/blacklist.txt");
    if(file.is_open()){
        string line;
        while(getline(file, line)){
            if(line == ip + ":" + to_string(port)){
                success = true;
            }
        }
        file.close();
    }
  return success;
} 
/********************************************************************************************/
bool isCached(element * ptr){

  string key = ptr -> destination_ip + ':' + to_string(ptr -> destination_port ) + ':' + ptr -> filename;
  if(cache_loc.find(key)==cache_loc.end())
    return false;
  char * dt = ctime(&cache_time[key][2]);
  ptr->date_time = &dt[0];
  return true;
}
void doCache(element *ptr , string response)
{
  time_t time_stamp;
  time(&time_stamp);
  string key = ptr -> destination_ip + ':' + to_string(ptr -> destination_port ) + ':' + ptr -> filename;
  //cout << cache_time[key].size() << endl;
  if(cache_time[key].size() <= 2)
    cache_time[key].push_back(time_stamp);
  else
  {
    // Pop first
  	cache_time[key].erase(cache_time[key].begin());
  	// Enter new value and compute
  	cache_time[key].push_back(time_stamp);
    double sum_time = difftime(cache_time[key][2],cache_time[key][1]) + difftime(cache_time[key][1],cache_time[key][0]);
    if(sum_time <= 300)
    {
      
      if(cache_loc.find(key)==cache_loc.end())
      //not present in cache
      {
        auto it = cache_loc.begin();
        int cnt = 0;
        for(auto i = it; i != cache_loc.end(); ++i)
        {
          if(i->second == (idx + 1))
          {
            it = i;
            break;
          }
        }
        // Add element to cache
        cache_loc[key] = it->second;

        // Pop first element
        cache_loc.erase(it); 
        idx++;
        idx = idx % 3;
        // Writing in the data
        FILE* fptr;
        if(cache_loc[key] == 1)
        {
          fptr = fopen("cache_data1.txt", "w+");
          if(fptr == NULL)
          {
             printf("Error!");
             exit(1);
          }
          fprintf(fptr,"%s", &response[0]);
          fclose(fptr);

        }

        else if(cache_loc[key] == 2)
        {
          fptr = fopen("cache_data2.txt", "w+");
          if(fptr == NULL)
          {
             printf("Error!");
             exit(1);
          }
          fprintf(fptr,"%s", &response[0]);
          fclose(fptr);
          
        }

        else if(cache_loc[key] == 3)
        {
          fptr = fopen("cache_data3.txt", "w+");
          if(fptr == NULL)
          {
             printf("Error!");
             exit(1);
          }
          fprintf(fptr,"%s", &response[0]);
          fclose(fptr);
          
        }

      } 

    }
  }
}
string cachedCopy(element * ptr){
  // string s = (ptr -> destination_ip) + ':' + to_string(ptr -> destination_port) + '/';
  //  return "";
  cout << "Using cached copy.\n";
  string key = ptr -> destination_ip + ':' + to_string(ptr -> destination_port ) + ':' + ptr -> filename;



  string cachedData;

  if(cache_loc[key] == 1)
  {
    FILE *fptr; 
    fptr = fopen("cache_data1.txt", "r"); 

    if (fptr == NULL) 
    { 
        printf("Cannot open file \n"); 
        exit(0); 
    } 
    
    // Read contents from file 
    char c = fgetc(fptr); 
    while (c != EOF) 
    { 
        cachedData.push_back(c);
        c = fgetc(fptr); 
    } 

  }

  if(cache_loc[key] == 2)
  {
    FILE *fptr; 
    fptr = fopen("cache_data2.txt", "r"); 

    if (fptr == NULL) 
    { 
        printf("Cannot open file \n"); 
        exit(0); 
    } 
    
    // Read contents from file 
    char c = fgetc(fptr); 
    while (c != EOF) 
    { 
        cachedData.push_back(c);
        c = fgetc(fptr); 
    } 

  }

  if(cache_loc[key] == 3)
  {
    FILE *fptr; 
    fptr = fopen("cache_data3.txt", "r"); 

    if (fptr == NULL) 
    { 
        printf("Cannot open file \n"); 
        exit(0); 
    } 
    
    // Read contents from file 
    char c = fgetc(fptr); 
    while (c != EOF) 
    { 
        cachedData.push_back(c);
        c = fgetc(fptr); 
    } 

  }

  return cachedData;



}
void update(element * ptr, string response){
  FILE* fptr;
  string key = ptr -> destination_ip + ':' + to_string(ptr -> destination_port ) + ':' + ptr -> filename;
  if(cache_loc[key] == 1)
  {
    fptr = fopen("cache_data1.txt", "w+");
    if(fptr == NULL)
    {
        printf("Error!");
        exit(1);
    }
    fprintf(fptr,"%s", &response[0]);
    fclose(fptr);

  }

  else if(cache_loc[key] == 2)
  {
    fptr = fopen("cache_data2.txt", "w+");
    if(fptr == NULL)
    {
        printf("Error!");
        exit(1);
    }
    fprintf(fptr,"%s", &response[0]);
    fclose(fptr);
    
  }

  else if(cache_loc[key] == 3)
  {
    fptr = fopen("cache_data3.txt", "w+");
    if(fptr == NULL)
    {
        printf("Error!");
        exit(1);
    }
    fprintf(fptr,"%s", &response[0]);
    fclose(fptr);
    
  }
  time_t time_stamp;
  time(&time_stamp);
  cache_time[key].erase(cache_time[key].begin());
  cache_time[key].push_back(time_stamp);
}
/*********************************************************************************************/
void * serveRequest(void * arg)
{
    int valread;
    time_t current_time;
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
    string response = "";
    current_time = time(0);
    char * dt = ctime(&current_time);
    ptr->date_time = &dt[0];
    ptr->date_time = "Tue Apr  2 12:59:30 2019";
    if(isBlackList(ptr->destination_ip, ptr->destination_port) && !ptr->isAuthenticated){
      response = "Authentication required to access the requested domain\n";
    }
    else{
    /********************************* Connect to Server ************************************/
      int server_socket = connect(ptr);
    /****************************************************************************************/
      if(isCached(ptr)){
        response = cacheControl(server_socket, ptr, http_request);
        if(!isModified(response))
        {
          response = cachedCopy(ptr);
        }
        else{
          // Update cached copy
          update(ptr , response);
        }
      }
      else{
    /************************** Send request to server and get response *********************/
        
        response = communication(server_socket, ptr, http_request);
        doCache(ptr , response);
    /****************************************************************************************/
      }
    }
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

    FILE *fptr1;
    fptr1 = fopen("cache_data1.txt", "w+");

    FILE *fptr2;
    fptr2 = fopen("cache_data2.txt", "w+");

    FILE *fptr3;
    fptr3 = fopen("cache_data3.txt", "w+");

    cache_loc["dummy_val"] = 1;
    cache_loc["dummy_val1"] = 2;
    cache_loc["dummy_val2"] = 3;





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
