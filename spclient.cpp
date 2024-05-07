#include "spclient.h"

#include <iostream>
#include <iomanip>
#include "sptype.h"
#include "sptupletype.h"


bool SPClient::start() {
    std::tuple startFrame{HeaderSize, FrameType::Start};
    sendTuple(startFrame, m_sendBuffer, m_stream);

    //logging
    std::cout << "Sending START\n";

    return isAck();
}


bool SPClient::stop() {
    std::tuple stopFrame{HeaderSize, FrameType::Stop};
    sendTuple(stopFrame, m_sendBuffer, m_stream);

    //logging
    std::cout << "Sending STOP\n";

    return isAck();
}


bool SPClient::GeneralInterrogation() {
    std::tuple gIFrame{HeaderSize, FrameType::GeneralInterrogation};
    sendTuple(gIFrame, m_sendBuffer, m_stream);
    std::cout << "Sending GI\n";

    if (isAck()) {
        while (receivePoints());
    }
}


bool SPClient::isAck() {
    std::tuple<FrameLength, FrameType> answerFrame;
    if (receiveTuple(answerFrame, m_recvBuffer, m_stream)) {
        FrameType frameType = std::get<1>(answerFrame);
        //logging
        switch (frameType) {
            case FrameType::Ack : std::cout << "ACT\n" << std::endl; break;
            case FrameType::Nack : std::cout << "Not ACT\n" << std::endl; break;
            default: std::cout << "Wrong answer!\n" << std::endl;
        }

        if (frameType == FrameType::Ack) return true;
    };
    return false;
}


void SPClient::readPoints(std::tuple<PointId, DigitValue, TimeTag, Quality> &point, size_t bufferLength) {
    if (m_recvBuffer.size() < bufferLength) return;
    size_t pointSize = sizeOfElems(point);
    for (size_t i = 0; i < bufferLength; i += pointSize) {
        readFromBuffer(m_recvBuffer, point, i);
        //logging
        const auto [pointId, value, timeTag, quality] = point;
        std::cout << "PointId = "   << pointId;
        std::cout << ", Value = "   << value;
        std::cout << ", TimeTag = " << std::put_time(std::gmtime(&timeTag), "%F %T");;
        std::cout << ", Quality = " << quality;
        std::cout << "\n";
    }
}


void SPClient::readPoints(std::tuple<PointId, AnalogValue , TimeTag, Quality> &point, size_t bufferLength) {
    if (m_recvBuffer.size() < bufferLength) return;
    size_t pointSize = sizeOfElems(point);
    for (size_t i = 0; i < bufferLength; i += pointSize) {
        readFromBuffer(m_recvBuffer, point, i);
        //logging
        const auto [pointId, value, timeTag, quality] = point;
        std::cout << "PointId = "   << pointId;
        std::cout << ", Value = "   << value;
        std::cout << ", TimeTag = " << std::put_time(std::gmtime(&timeTag), "%F %T");;
        std::cout << ", Quality = " << quality;
        std::cout << "\n";
    }
}


bool SPClient::receivePoints() {
    std::tuple<FrameLength, FrameType> headerFrame;
    if (!receiveTuple(headerFrame, m_recvBuffer, m_stream)) return false;
    const auto [frameLength, frameType] = headerFrame;
    std::tuple<TransmissionType, SignalCount> signalHeader;
    switch (frameType) {
        case FrameType::DigitalPoints :
        case FrameType::AnalogPoints :
            receiveTuple(signalHeader, m_recvBuffer, m_stream);
            break;
        default:
            return false;
    }

    size_t lenghtPoints = frameLength - sizeOfElems(headerFrame) - sizeOfElems(signalHeader);
    m_recvBuffer.resize(lenghtPoints);
    m_stream->receiveAll(&m_recvBuffer.front(), lenghtPoints, 0);

    switch (frameType) {
        case FrameType::DigitalPoints : {
            std::tuple<PointId, DigitValue, TimeTag, Quality> digitPoint;
            readPoints(digitPoint, lenghtPoints);
            break;
        }
        case FrameType::AnalogPoints : {
            std::tuple<PointId, AnalogValue, TimeTag, Quality> analogPoint;
            readPoints(analogPoint, lenghtPoints);
            break;
        }
    }

    return true;
}

