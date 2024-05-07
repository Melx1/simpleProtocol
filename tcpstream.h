/*
   TCPStream.h
   TCPStream class interface. TCPStream provides methods to trasnfer
   data between peers over a TCP/IP connection.
*/

#ifndef SIMPLEPROTOCOL_TCPSTREAM_H
#define SIMPLEPROTOCOL_TCPSTREAM_H

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>


class TCPStream
{
    int          m_sd;
    std::string  m_peerIP;
    int          m_peerPort;

  public:
    friend class TCPConnector;

    ~TCPStream();

    ssize_t send(const void* buffer, size_t len, int flags);
    ssize_t sendAll(uint8_t* buffer, size_t len, int flags);
    ssize_t receive(void* buffer, size_t len, int flags, int timeout = 0);
    ssize_t receiveAll(uint8_t* buffer, size_t len, int flags, int timeout = 0);

    std::string getPeerIP();
    int getPeerPort();

    enum {
        connectionClosed = 0,
        connectionReset = -1,
        connectionTimedOut = -2
    };

  private:
    bool waitForReadEvent(int timeout);

    TCPStream(int sd, struct sockaddr_in* address);
    TCPStream(const TCPStream& stream);
};

#endif
