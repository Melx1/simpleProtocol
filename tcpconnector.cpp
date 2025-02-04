/*
   TCPConnector class definition. TCPConnector provides methods to actively
   establish TCP/IP connections with a server.
*/
#include "tcpconnector.h"

#include <cstdio>
#include <cstring>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cerrno>


TCPStream* TCPConnector::connect(const char* server, int port)
{
    struct sockaddr_in address{};

    memset (&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (resolveHostName(server, &(address.sin_addr)) != 0 ) {
        inet_pton(PF_INET, server, &(address.sin_addr));        
    } 

    // Create and connect the socket, bail if we fail in either case
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("socket() failed");
        return nullptr;
    }
    if (::connect(sd, (struct sockaddr*)&address, sizeof(address)) != 0) {
        perror("connect() failed");
        close(sd);
        return nullptr;
    }
    return new TCPStream(sd, &address);
}

TCPStream* TCPConnector::connect(const char* server, int port, int timeout)
{
    if (timeout == 0) return connect(server, port);
    
    struct sockaddr_in address{};

    memset (&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (resolveHostName(server, &(address.sin_addr)) != 0 ) {
        inet_pton(PF_INET, server, &(address.sin_addr));        
    }     

    long arg;
    fd_set sdset;
    struct timeval tv{};
    socklen_t len;
    int result;
    int valopt;
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    
    // Bail if we fail to create the socket
    if (sd < 0) {
        perror("socket() failed");
        return nullptr;
    }    

    // Set socket to non-blocking
    arg = fcntl(sd, F_GETFL, NULL);
    arg |= O_NONBLOCK;
    fcntl(sd, F_SETFL, arg);
    
    // Connect with time limit
    std::string message;
    if ((result = ::connect(sd, (struct sockaddr *)&address, sizeof(address))) < 0) {
        if (errno == EINPROGRESS) {
            tv.tv_sec = timeout;
            tv.tv_usec = 0;
            FD_ZERO(&sdset);
            FD_SET(sd, &sdset);
            int s;
            do {
                s = select(sd + 1, nullptr, &sdset, nullptr, &tv);
            } while (s == -1 && errno == EINTR);
            if (s > 0) {
                len = sizeof(int);
                getsockopt(sd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &len);
                if (valopt) {
                    fprintf(stderr, "connect() error %d - %s\n", valopt, strerror(valopt));
                }
                // connection established
                else result = 0;
            }
            else {
                close(sd);
                fprintf(stderr, "connect() timed out\n");
	        }
        }
        else {
            close(sd);
            fprintf(stderr, "connect() error %d - %s\n", errno, strerror(errno));
        }
    }

    // Return socket to blocking mode 
    arg = fcntl(sd, F_GETFL, NULL);
    arg &= (~O_NONBLOCK);
    fcntl(sd, F_SETFL, arg);

    // Create stream object if connected
    if (result == -1) return nullptr;
    return new TCPStream(sd, &address);
}

int TCPConnector::resolveHostName(const char* hostname, struct in_addr* addr) {
    struct addrinfo *res;
  
    int result = getaddrinfo (hostname, nullptr, nullptr, &res);
    if (result == 0) {
        memcpy(addr, &((struct sockaddr_in *) res->ai_addr)->sin_addr, sizeof(struct in_addr));
        freeaddrinfo(res);
    }
    return result;
}
