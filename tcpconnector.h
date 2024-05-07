/*
   TCPConnector class interface. TCPConnector provides methods to actively
   establish TCP/IP connections with a server.
*/

#ifndef SIMPLEPROTOCOL_TCPCONNECTOR_H
#define SIMPLEPROTOCOL_TCPCONNECTOR_H

#include <netinet/in.h>
#include "tcpstream.h"

class TCPConnector
{
  public:
    TCPStream* connect(const char* server, int port);
    TCPStream* connect(const char* server, int port, int timeout);

    
  private:
    int resolveHostName(const char* host, struct in_addr* addr);
};

#endif
