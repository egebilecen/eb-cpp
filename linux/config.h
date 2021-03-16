#pragma once
#include <string>

struct SERVER_CONFIG
{
    inline static std::string const ipAddr = "localhost";
    inline static uint16_t    const portNo = 6969;
};

struct SERIALPORT_CONFIG
{
    inline static std::string const portName = "/dev/pts/3";
    inline static uint32_t    const baudrate = 57600;
};
