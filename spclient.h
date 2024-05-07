#ifndef SIMPLEPROTOCOL_SPCLIENT_H
#define SIMPLEPROTOCOL_SPCLIENT_H

#include <ostream>
#include <vector>
#include <utility>
#include <tuple>
#include <cstring>
#include <variant>
#include "tcpstream.h"
#include "sptype.h"


class SPClient {
public:
    explicit SPClient (TCPStream* stream) : m_stream(stream) {};
    bool start ();
    bool stop ();
    bool GeneralInterrogation();

private:
    bool isAck ();
    void readPoints (std::tuple <PointId, DigitValue, TimeTag, Quality>& point, size_t bufferLength);
    void readPoints (std::tuple <PointId, AnalogValue , TimeTag, Quality>& point, size_t bufferLength);
    bool receivePoints();

private:
    TCPStream* m_stream;
    std::vector<uint8_t> m_sendBuffer;
    std::vector<uint8_t> m_recvBuffer;
};

#endif //SIMPLEPROTOCOL_SPCLIENT_H
