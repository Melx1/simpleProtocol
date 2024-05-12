cmake_minimum_required(VERSION 3.25)
project(simpleProtocol)

set(CMAKE_CXX_STANDARD 20)

add_executable(simpleProtocol   main.cpp
        tcpstream.h
        tcpstream.cpp
        tcpconnector.h
        tcpconnector.cpp
        sptype.h
        spclient.h
        spclient.cpp
        sptupletype.h
        sptype.cpp
        tsqueue.h)
