#ifndef SIMPLEPROTOCOL_SPTYPE_H
#define SIMPLEPROTOCOL_SPTYPE_H

#include <bitset>
#include <vector>
#include <ctime>
#include <cstdint>


using FrameLength = uint32_t;
using PointId     = uint32_t;
using DigitValue  = uint8_t;
using AnalogValue = float;
using TimeTag     = std::time_t;
using SignalCount = uint16_t;

std::ostream& operator<<(std::ostream& os, DigitValue obj);

enum class FrameType : uint8_t {
     DigitalPoints        = 1
    ,AnalogPoints         = 2
    ,DigitalControl       = 3
    ,Start                = 4
    ,Stop                 = 5
    ,GeneralInterrogation = 6
    ,Ack                  = 7
    ,Nack                 = 8
};

enum class Quality : uint8_t {
     Valid       = 0 //лучше использовать для флага другое значение, т.к. проверка & не совсем корректна
    ,Substituted = 1
    ,Overflow    = 2
};

Quality operator|(Quality lhs, Quality rhs);
Quality operator&(Quality lhs, Quality rhs);
std::ostream& operator<<(std::ostream& os, const Quality& obj);

enum class TransmissionType : uint8_t {
    Interrogation = 1
    ,Spontaneous   = 2
};

//length is count of byte
constexpr size_t HeaderSize = sizeof(FrameLength) + sizeof(FrameType);


#endif //SIMPLEPROTOCOL_SPTYPE_H
