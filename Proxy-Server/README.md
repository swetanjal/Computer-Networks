# Authenticated Users:
- swetanjal:swet
- shreyas:hack_shrey

# Blacklisted servers:
- 127.0.0.1:20200
- 127.0.0.1:20199

# Start the servers:
- Change directory to 'servers'
- Change directory to each of the folder. Eg:'127.0.0.1:20000'
- Run command: python2 server.py port_no
where port_no is the port number of the server in whose directory you are in

# Start Proxy server:
- Run command: g++ proxy-server.cpp -o proxy-server -lpthread
- Run command: ./proxy-server

# Sending requests:
Send curl requests of the form:
- curl --request GET -u username:password --proxy 127.0.0.1:20100 --local-port 20010 127.0.0.1:20200/1.data
- curl --request GET --proxy 127.0.0.1:20100 --local-port 20010 127.0.0.1:20190/1.data
- curl --request POST --proxy 127.0.0.1:20100 --local-port 20010 127.0.0.1:20190/1.data

# Features:
- Implementation in C++ with most efficient data structures to serve requests on a large scale very fast.
- Round robin caching implemented. If a webpage is rquested more than 3 times in 5 minutes, it is cached and next time before using the cached copy the 'If-Modified-Since' header is used to verify with the server that the cached copy is up to date or not.
- Supports blacklisting of servers. In order to access blacklisted servers, authentication required.
- Supports POST requests as well.

# Scope for improvement:
- Add reverse proxy feature by which requests can be served to outside IIIT servers by authenticating users.
- Modify Round Robin Approach to Least Recently Requested cached response.
- The server cannot process HTTP requests of size greater than 1000000 characters.
