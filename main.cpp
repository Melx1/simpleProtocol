#include <string>
#include "tcpconnector.h"
#include "spclient.h"


int main() {
    std::string host = "cpptest.08z.ru";
    int port = 12567;

    auto* connector = new TCPConnector();
    TCPStream* stream = connector->connect(host.c_str(), port);

    if (stream) {
        SPClient spclient (stream);
        //init client
        spclient.start();

        spclient.generalInterrogation();

        //control
        spclient.digitalControl(2, 1);

        //stop
        spclient.stop();
    }
    delete stream;
    return 0;

}



