#ifndef serial_port_h
#define serial_port_h

#include <string>
#include <cstring>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include "../Logger/logger.h"

namespace EB
{
    class SerialPort
    {
    //--------------------------------- Public Declerations
    public:
        //--------------------------------- Type Defines
        typedef speed_t baudrate_t;

        //--------------------------------- Constructor & Destructor Method
        SerialPort(std::string const& port_name,
                   baudrate_t         baudrate);
        ~SerialPort();

        //--------------------------------- Setter Method(s)
        void set_buffer_size(size_t buffer_size);

        //--------------------------------- Getter Method(s)
        int get_last_error_code();

        //--------------------------------- Serial Port Methods
        void open();
        void close();
        void write(std::string const& data);
        std::string read();

    //--------------------------------- Private Declerations
    private:
        //--------------------------------- Variables
        //---------------- Serial Port
        baudrate_t         baudrate;
        std::string const* port_name;
        int                serial_port;

        //---------------- Buffer
        char*  buffer      = nullptr;
        size_t buffer_size = 1025;

        //---------------- Codes
        int status_code   = 0;
        int last_err_code = 0;

        //--------------------------------- Methods
        baudrate_t get_baudrate(uint32_t baud);
    };
}

#endif // serial_port_h
