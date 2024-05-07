#ifndef SIMPLEPROTOCOL_SPTUPLETYPE_H
#define SIMPLEPROTOCOL_SPTUPLETYPE_H

#include <tuple>
#include <variant>
#include <cstring>
#include "sptype.h"
#include "tcpstream.h"


//tuple to buffer
    template <typename TupleT, std::size_t... Is>
    void writeToBufferImp(std::vector<uint8_t> &buffer, const TupleT& tp, std::index_sequence<Is...>) {
        std::size_t written = 0;
        auto writeElem = [&written, &buffer](const auto& val) {
            size_t count = sizeof (val);
            buffer.resize(written + count);
            //convert val from LE to BE, if necessary
            std::memcpy(&buffer.at(written), &val, count);
            written += count;
        };

        (writeElem(std::get<Is>(tp)), ...);
    }

    template <typename TupleT, std::size_t TupSize = std::tuple_size<TupleT>::value>
    void writeToBuffer(std::vector<uint8_t> &buffer, const TupleT& tp) {
        writeToBufferImp(buffer, tp, std::make_index_sequence<TupSize>{});
    }


//tuple from buffer
    template <typename TupleT, std::size_t... Is>
    void readFromBufferImp(const std::vector<uint8_t> &buffer, TupleT& tp, std::index_sequence<Is...>, size_t startPos) {
        std::size_t read = startPos;
        auto readElem = [&read, &buffer](auto& val) {
            size_t count = sizeof (val);
            //convert val from BE to LE, if necessary
            std::memcpy(&val, &buffer.at(read), count);
            read += count;
        };

        (readElem(std::get<Is>(tp)), ...);
    }

    template <typename TupleT, std::size_t TupSize = std::tuple_size<TupleT>::value>
    void readFromBuffer(const std::vector<uint8_t> &buffer, TupleT& tp, size_t startPos = 0) {
        readFromBufferImp(buffer, tp, std::make_index_sequence<TupSize>{}, startPos);
    }


//size of elems
    template <typename TupleT, std::size_t... Is>
    size_t sizeOfElemsImp(const TupleT& tp, std::index_sequence<Is...>) {
        std::size_t size = 0;
        auto sizeOfElem = [&size](const auto& val) {
            size += sizeof (val);
        };
        (sizeOfElem(std::get<Is>(tp)), ...);
        return size;
    }

    template <typename TupleT, std::size_t TupSize = std::tuple_size<TupleT>::value>
    size_t sizeOfElems(const TupleT& tp) {
        return sizeOfElemsImp(tp, std::make_index_sequence<TupSize>{});
    }


//send tuple to stream
    template <typename TupleT>
    bool sendTuple(const TupleT &tp, std::vector<uint8_t> &sendBuffer, TCPStream* stream) {
        writeToBuffer(sendBuffer, tp);
        size_t msgLen = sizeOfElems(tp);
        ssize_t N = stream->sendAll(&sendBuffer.front(), msgLen, 0);
        if (N == msgLen) return true;
        return false;
    }


//receive tuple from stream
    template <typename TupleT>
    bool receiveTuple(TupleT &tp, std::vector<uint8_t> &receiveBuffer, TCPStream* stream) {
        size_t msgLen = sizeOfElems(tp);
        receiveBuffer.resize(msgLen);
        ssize_t N = stream->receiveAll(&receiveBuffer.front(), msgLen, 0);
        if (N == msgLen) {
            readFromBuffer( receiveBuffer, tp);
            return true;
        }
        return false;
    }

#endif //SIMPLEPROTOCOL_SPTUPLETYPE_H
