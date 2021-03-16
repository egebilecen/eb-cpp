#include <iostream>
#include "EB/TCP/server.h"
#include "EB/Protocol/protocol.h"
#include "EB/SerialPort/serial_port.h"
#include "config.h"
#include "handlers.h"

int main()
{
    /* TCP Server */
//    EB::Protocol::EBP protocol;
//    protocol.add_handler("hello", &hello);
//
//    EB::TCP::Server server(&SERVER_CONFIG::ipAddr, &SERVER_CONFIG::portNo, &protocol, true);
//    server.start();

    /* Serial Port */
    EB::SerialPort::USB sp(SERIALPORT_CONFIG::portName, SERIALPORT_CONFIG::baudrate);
    sp.open();

    std::string data2write = "Hello serial port!";
    sp.write(data2write);
}
