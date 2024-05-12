#include "spclient.h"

#include <iostream>
#include <iomanip>
#include <ranges>
#include <thread>
#include "sptype.h"
#include "sptupletype.h"


bool SPClient::start() {
    std::tuple<FrameLength, FrameType> startFrame{HeaderSize, FrameType::Start};
    SPTuple::sendTuple(startFrame, m_sendBuffer, m_stream);

    //logging
    printMsg("Sending START");

    std::tuple<FrameLength, FrameType> headerFrame;
    if (SPTuple::receiveTuple(headerFrame, m_recvBuffer, m_stream)) {
        if (isAck(headerFrame)) {
            m_listenerActive = true;
            m_listener = std::thread([this](){
                while(m_listenerActive) {
                    listen();
                }
            });
            return true;
        }
    }

    return false;
}


bool SPClient::stop() {
    std::tuple<FrameLength, FrameType> stopFrame{HeaderSize, FrameType::Stop};
    SPTuple::sendTuple(stopFrame, m_sendBuffer, m_stream);

    //logging
    printMsg("Sending STOP");

    std::tuple<FrameLength, FrameType> headerFrame;
    SPTuple::readFromQueue(m_recvQueue, headerFrame);
    if (isAck(headerFrame)) {
        m_listenerActive = false; // stop listener;
        m_listener.join();
        return true;
    }
    return false;
}


bool SPClient::generalInterrogation() {
    std::tuple<FrameLength, FrameType> gIFrame{HeaderSize, FrameType::GeneralInterrogation};
    SPTuple::sendTuple(gIFrame, m_sendBuffer, m_stream);
    //logging
    printMsg("Sending GI");

    std::tuple<FrameLength, FrameType> headerFrame;
    SPTuple::readFromQueue(m_recvQueue, headerFrame);
    if (isAck(headerFrame)) {
        readPointsFrameFromQueue();
        readPointsFrameFromQueue();
        return true;
    }
    return false;
}


bool SPClient::isAck(const std::tuple<FrameLength, FrameType>& headerFrame) {
    FrameType frameType = std::get<1>(headerFrame);
    //logging
    switch (frameType) {
        case FrameType::Ack  : printMsg("ACT\n"); break;
        case FrameType::Nack : printMsg("Not ACT\n"); break;
        default              : printMsg("Wrong type!\n");
    }

    if (frameType == FrameType::Ack) return true;
    return false;
}


bool SPClient::digitalControl(PointId id, DigitValue value) {
    std::tuple<FrameLength, FrameType, PointId, DigitValue> controlFrame {SPTuple::sizeOfElems(controlFrame), FrameType::DigitalControl, id, value};
    SPTuple::sendTuple(controlFrame, m_sendBuffer, m_stream);

    //logging
    printMsg("Sending DIGITAL_CONTROL");

    std::tuple<FrameLength, FrameType> headerFrame;
    SPTuple::readFromQueue(m_recvQueue, headerFrame);
    return isAck(headerFrame);
}

void SPClient::listen() {
    std::tuple<FrameLength, FrameType> headerFrame;
    if (SPTuple::receiveTuple(headerFrame, m_recvBuffer, m_stream, 10)) {
        //receive
        auto [frameLength, frameType] = headerFrame;
        if (frameLength > HeaderSize) {
            size_t tailSize = frameLength - HeaderSize;
            m_recvBuffer.resize(tailSize);
            if (!m_stream->receiveAll(&m_recvBuffer.front(), tailSize, 0)) {
                printMsg("Receive error!\n");
                return;
            }
        }

        std::tuple<TransmissionType, SignalCount> signalHeader;
        //push to queue
        switch (frameType) {
            case FrameType::DigitalPoints :
            case FrameType::AnalogPoints : {
                SPTuple::readFromBuffer( m_recvBuffer, signalHeader);
                auto transmissionType = std::get<0>(signalHeader);
                if (transmissionType == TransmissionType::Interrogation) {
                    SPTuple::pushToQueue(m_recvQueue, headerFrame);
                    m_recvQueue.push(m_recvBuffer);
                    return;
                }
                break;
            }
            case FrameType::Ack :
            case FrameType::Nack :
                SPTuple::pushToQueue(m_recvQueue, headerFrame);
                return;
            default:
                printMsg("Wrong type!\n" );
                return;
        }

        //spontaneous signals processing
        switch (frameType) {
            case FrameType::DigitalPoints : {
                std::tuple<PointId, DigitValue, TimeTag, Quality> digitPoint;
                readPointsFromBuffer(digitPoint, SPTuple::sizeOfElems(signalHeader));
                return;
            }
            case FrameType::AnalogPoints : {
                std::tuple<PointId, AnalogValue , TimeTag, Quality> analogPoint;
                readPointsFromBuffer(analogPoint, SPTuple::sizeOfElems(signalHeader));
                return;
            }
        }
    }
    else {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool SPClient::readPointsFrameFromQueue() {
    std::tuple<FrameLength, FrameType> headerFrame;
    SPTuple::readFromQueue(m_recvQueue, headerFrame);
    auto [frameLength, frameType] = headerFrame;
    std::tuple<TransmissionType, SignalCount> signalHeader;
    SPTuple::readFromQueue(m_recvQueue, signalHeader);
    switch (frameType) {
        case FrameType::DigitalPoints : {
            std::tuple<PointId, DigitValue, TimeTag, Quality> digitPoint;
            size_t count = (frameLength - SPTuple::sizeOfElems(headerFrame) - SPTuple::sizeOfElems(signalHeader))/(SPTuple::sizeOfElems(digitPoint));
            readPointsFromQueue(digitPoint, count);
            return true;
        }
        case FrameType::AnalogPoints : {
            std::tuple<PointId, AnalogValue, TimeTag, Quality> digitPoint;
            size_t count = (frameLength - SPTuple::sizeOfElems(headerFrame) - SPTuple::sizeOfElems(signalHeader))/(SPTuple::sizeOfElems(digitPoint));
            readPointsFromQueue(digitPoint, count);
            return true;
        }
        default:
            //logging
            printMsg("Wrong type!\n");
            return false;
    }
}

void SPClient::printMsg(const std::string_view &msg) {
    std::lock_guard<std::mutex> lock(m_lockPrint);
    std::cout << msg << std::endl;
}

void SPClient::printMsg() {
    std::lock_guard<std::mutex> lock(m_lockPrint);
    std::cout << std::endl;
}


template<typename T>
void SPClient::readPointsFromQueue(std::tuple<PointId, T, TimeTag, Quality> &point, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        SPTuple::readFromQueue(m_recvQueue, point);
        //logging
        printSignal(point);
    }
    //logging
    printMsg("");
}
template void SPClient::readPointsFromQueue(std::tuple<PointId, DigitValue, TimeTag, Quality> &point, size_t count);
template void SPClient::readPointsFromQueue(std::tuple<PointId, AnalogValue, TimeTag, Quality> &point, size_t count);


template<typename T>
size_t SPClient::readPointsFromBuffer(std::tuple<PointId, T, TimeTag, Quality> &point, size_t startPos) {
    size_t pointSize = SPTuple::sizeOfElems(point);
    size_t i;
    for (i = startPos; i + pointSize <= m_recvBuffer.size(); i += pointSize) {
        SPTuple::readFromBuffer(m_recvBuffer, point, i);
        //logging
        printSignal(point);
    }
    //logging
    printMsg();
    return i;
}

template size_t SPClient::readPointsFromBuffer(std::tuple<PointId, DigitValue, TimeTag, Quality> &point, size_t bufferLength);
template size_t SPClient::readPointsFromBuffer(std::tuple<PointId, AnalogValue, TimeTag, Quality> &point, size_t bufferLength);


template<typename T>
void SPClient::printSignal(const std::tuple<PointId, T, TimeTag, Quality> &signal) {
    const auto [pointId, value, timeTag, quality] = signal;
    std::lock_guard<std::mutex> lock (m_lockPrint);
    std::cout << "PointId = "   << pointId;
    std::cout << ", Value = "   << value;
    std::cout << ", TimeTag = " << std::put_time(std::gmtime(&timeTag), "%F %T");
    std::cout << ", Quality = " << quality;
    std::cout << "\n";
}

template void SPClient::printSignal(const std::tuple<PointId, DigitValue, TimeTag, Quality> &signal);
template void SPClient::printSignal(const std::tuple<PointId, AnalogValue , TimeTag, Quality> &signal);

