/*
   TCPStream class definition. TCPStream provides methods to transfer
   data between peers over a TCP/IP connection.
*/
#include "tcpstream.h"

#include <arpa/inet.h>

TCPStream::TCPStream(int sd, struct sockaddr_in* address) : m_sd(sd) {
    char ip[50];
    inet_ntop(PF_INET, (struct in_addr*)&(address->sin_addr.s_addr), ip, sizeof(ip)-1);
    m_peerIP = ip;
    m_peerPort = ntohs(address->sin_port);
}

TCPStream::~TCPStream() {
    close(m_sd);
}

ssize_t TCPStream::send(const void* buffer, size_t len, int flags) {
    return ::send(m_sd, buffer, len, flags);
}

ssize_t TCPStream::sendAll(uint8_t *buffer, size_t len, int flags) {
    size_t total = 0;
    size_t n;

    while(total < len) {
        n = ::send(m_sd, buffer + total, len - total, flags);
        if (n == -1) break;
        total += n;
    }

    return n == -1 ? -1 : total;
}

ssize_t TCPStream::receive(void* buffer, size_t len,  int flags, int timeout) {
    if (timeout <= 0)
        return ::recv(m_sd, buffer, len, flags);

    if (waitForReadEvent(timeout))
        return ::recv(m_sd, buffer, len, flags);

    return connectionTimedOut;
}

ssize_t TCPStream::receiveAll(uint8_t *buffer, size_t len, int flags, int timeout) {
    auto m_recive = [&](uint8_t *buffer, size_t len, int flags) {
        ssize_t total = 0;
        ssize_t received = 0;
        while(total < len) {
            received = ::recv(m_sd,buffer + total, len - total, flags);
            if (received <= 0) break;
            total += received;
        }
        return received <=0 ? received : total;
    };

    if (timeout <= 0) {
        return m_recive(buffer, len, flags);
    }

    if (waitForReadEvent(timeout)) {
        return m_recive(buffer, len, flags);
    }

    return connectionTimedOut;
}

std::string TCPStream::getPeerIP() {
    return m_peerIP;
}

int TCPStream::getPeerPort() {
    return m_peerPort;
}

bool TCPStream::waitForReadEvent(int timeout) {
    fd_set sdset;
    struct timeval tv{};

    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    FD_ZERO(&sdset);
    FD_SET(m_sd, &sdset);
    if (select(m_sd+1, &sdset, nullptr, nullptr, &tv) > 0) {
        return true;
    }
    return false;
}
