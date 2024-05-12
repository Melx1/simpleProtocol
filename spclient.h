#ifndef SIMPLEPROTOCOL_SPCLIENT_H
#define SIMPLEPROTOCOL_SPCLIENT_H

#include <vector>
#include <tuple>
#include <thread>
#include <mutex>
#include <atomic>
#include "tcpstream.h"
#include "sptype.h"
#include "sptupletype.h"
#include "tsqueue.h"

class SPClient {
public:
    explicit SPClient (TCPStream* stream) : m_stream(stream) {};
    bool start ();
    bool stop ();
    bool generalInterrogation();
    bool digitalControl(PointId id, DigitValue value);
    void listen();

private:

    bool isAck (const std::tuple<FrameLength, FrameType>& headerFrame);
    bool readPointsFrameFromQueue(); //read frame header, signal header and points

    template<typename T>
    size_t readPointsFromBuffer (std::tuple <PointId, T, TimeTag, Quality>& point, size_t bufferLength);

    template<typename T>
    void readPointsFromQueue (std::tuple <PointId, T, TimeTag, Quality>& point, size_t count);

    template<typename T>
    void printSignal(const std::tuple<PointId, T, TimeTag, Quality> &signal);

    void printMsg (const std::string_view& msg);
    void printMsg ();

private:

    TCPStream* m_stream;
    std::vector<uint8_t> m_sendBuffer;
    std::vector<uint8_t> m_recvBuffer;
    TSQueue<uint8_t> m_recvQueue;
    std::thread m_listener;
    std::atomic<bool> m_listenerActive;
    std::mutex m_lockPrint;
};

#endif //SIMPLEPROTOCOL_SPCLIENT_H