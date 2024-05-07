

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <ctime>
#include <iomanip>
#include "tcpconnector.h"
#include "sptype.h"




int main() {
    std::string host = "cpptest.08z.ru";
    int port = 12567;

    auto* connector = new TCPConnector();
    TCPStream* stream = connector->connect(host.c_str(), port);

    if (stream) {
        std::vector<uint8_t> sendBuffer;
        std::vector<uint8_t> recvBuffer;

        //start
        {
            std::tuple startFrame{HeaderSize, FrameType::Start};
            writeToBuffer(sendBuffer, startFrame);
            stream->sendAll(&sendBuffer.front(), HeaderSize, 0);
            std::cout << "Sending START\n";
        }

        {
            std::tuple<FrameLength, FrameType> answerFrame;
            size_t messageLen = sizeOfElems(answerFrame);
            recvBuffer.resize(messageLen);
            stream->receiveAll(&recvBuffer.front(), messageLen, 0);
            readFromBuffer( recvBuffer, answerFrame);
            const auto [len, frameType] = answerFrame;
            switch (frameType) {
                case FrameType::Ack : std::cout << "ACT\n" << std::endl; break;
                case FrameType::Nack : std::cout << "Not ACT\n" << std::endl; break;
                default: std::cout << "Wrong answer!\n" << std::endl;
            }
        }

        //GeneralInterrogation
        {
            std::tuple gIFrame{HeaderSize, FrameType::GeneralInterrogation};
            writeToBuffer(sendBuffer, gIFrame);
            stream->sendAll(&sendBuffer.front(), HeaderSize, 0);
            std::cout << "Sending GI\n";
        }

        {
            std::tuple<uint32_t, FrameType> answerFrame;
            size_t messageLen = sizeOfElems(answerFrame);
            recvBuffer.resize(messageLen);
            stream->receiveAll(&recvBuffer.front(), messageLen, 0);
            readFromBuffer( recvBuffer, answerFrame);
            const auto [len, frameType] = answerFrame;
            switch (frameType) {
                case FrameType::Ack : std::cout << "ACT\n" << std::endl; break;
                case FrameType::Nack : std::cout << "Not ACT\n" << std::endl; break;
                default: std::cout << "Wrong answer!\n" << std::endl;
            }
        }

        { //receive digit
            std::tuple<uint32_t, FrameType, TransmissionType, uint16_t> signalHeaderFrame;
            size_t messageLen = sizeOfElems(signalHeaderFrame);
            recvBuffer.resize(messageLen);
            stream->receiveAll(&recvBuffer.front(), messageLen, 0);
            readFromBuffer( recvBuffer, signalHeaderFrame);
            const auto [len, frameType, transmissionType, count] = signalHeaderFrame;
            std::cout << "Receiving DIGITAL_POINTS\n";
            for (size_t i = 0; i < count; ++i) {
                std::tuple<uint32_t, uint8_t, std::time_t, Quality> digitSignal;
                size_t messageLen = sizeOfElems(digitSignal);
                recvBuffer.resize(messageLen);
                stream->receiveAll(&recvBuffer.front(), messageLen, 0);
                readFromBuffer( recvBuffer, digitSignal);
                const auto [pointId, value, timeTag, quality] = digitSignal;
                std::cout << "PointId = " << pointId;
                std::cout << ", Value = " << static_cast<int>(value);
                std::cout << ", TimeTag = " << std::put_time(std::gmtime(&timeTag), "%F %T");
                std::cout << ", Quality = " << quality;
                std::cout << "\n";
            }
            std::cout << std::endl;
        }

        { //receive analog
            std::tuple<uint32_t, FrameType, TransmissionType, uint16_t> signalHeaderFrame;
            size_t messageLen = sizeOfElems(signalHeaderFrame);
            recvBuffer.resize(messageLen);
            stream->receiveAll(&recvBuffer.front(), messageLen, 0);
            readFromBuffer( recvBuffer, signalHeaderFrame);
            const auto [len, frameType, transmissionType, count] = signalHeaderFrame;
            std::cout << "Receiving ANALOG_POINTS\n";
            for (size_t i = 0; i < count; ++i) {
                std::tuple<uint32_t, float, std::time_t, Quality> analogSignal;
                size_t messageLen = sizeOfElems(analogSignal);
                recvBuffer.resize(messageLen);
                stream->receiveAll(&recvBuffer.front(), messageLen, 0);
                readFromBuffer( recvBuffer, analogSignal);
                const auto [pointId, value, timeTag, quality] = analogSignal;
                std::cout << "PointId = " << pointId;
                std::cout << ", Value = " << value;
                std::cout << ", TimeTag = " << std::put_time(std::gmtime(&timeTag), "%F %T");
                std::cout << ", Quality = " << quality;
                std::cout << "\n";
            }
            std::cout << std::endl;
        }


        //Stop
        {
            std::tuple stopFrame{HeaderSize, FrameType::Stop};
            writeToBuffer(sendBuffer, stopFrame);
            stream->sendAll(&sendBuffer.front(), HeaderSize, 0);
            std::cout << "Sending STOP\n";
        }

        {
            std::tuple<uint32_t, FrameType> answerFrame;
            size_t messageLen = sizeOfElems(answerFrame);
            recvBuffer.resize(messageLen);
            stream->receiveAll(&recvBuffer.front(), messageLen, 0);
            readFromBuffer( recvBuffer, answerFrame);
            const auto [len, frameType] = answerFrame;
            switch (frameType) {
                case FrameType::Ack : std::cout << "ACT\n" << std::endl; break;
                case FrameType::Nack : std::cout << "Not ACT\n" << std::endl; break;
                default: std::cout << "Wrong answer!\n" << std::endl;
            }
        }
    }
    delete stream;
    return 0;

}



