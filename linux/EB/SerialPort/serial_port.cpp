#include "serial_port.h"

namespace EB
{
    //--------------------------------- Private Method Definations
    SerialPort::baudrate_t SerialPort::get_baudrate(uint32_t baud)
    {
        switch(baud)
        {
            case 9600  : return B9600  ;  break;
            case 57600 : return B57600 ;  break;
            case 115200: return B115200;  break;
            default    :
                Logger::Log("Given baudrate ("+to_string(baud)+") is not on list. Switching to default (9600) baudrate.",
                    __FILE__, __LINE__,
                    Logger::LOG_TYPE::WARNING);
                return B9600;
            break;
        }
    }

    //--------------------------------- Public Method Definations
    //---------------- Constructor
    SerialPort::SerialPort(std::string const&   port_name,
                         SerialPort::baudrate_t baudrate)
    {
        this->port_name   = &port_name;
        this->baudrate    =  this->get_baudrate(baudrate);
        this->buffer_size =  SerialPort::buffer_size;
    }

    //---------------- Destructor
    SerialPort::~SerialPort()
    {
        if(this->buffer != nullptr) delete[] this->buffer;
    }

    //---------------- Setter Methods
    void SerialPort::set_buffer_size(size_t buffer_size)
    {
        Logger::Log("Setting buffer size to "+to_string(buffer_size)+".",
                    __FILE__, __LINE__);
        if(this->status_code != 0)
        {
            Logger::Log("Cannot set the buffer size. Status code is not 0.",
                        __FILE__, __LINE__,
                        Logger::LOG_TYPE::ERROR);
            return;
        }

        this->buffer_size = buffer_size;

        Logger::Log("Buffer size successfully set to "+to_string(buffer_size)+".",
                    __FILE__, __LINE__);
    }

    //---------------- Getter Methods
    int SerialPort::get_last_error_code()
    {
        return this->last_err_code;
    }

    //---------------- Other Methods
    void SerialPort::open()
    {
        if(this->status_code != 0)
        {
            Logger::Log("Cannot open the serial port. Status code is not 0.",
                        __FILE__, __LINE__,
                        Logger::LOG_TYPE::ERROR);
            return;
        }


        Logger::Log("Getting serial port file descriptor.",
                    __FILE__, __LINE__);
        this->serial_port = ::open(this->port_name->c_str(), O_RDWR);

        if(this->serial_port < 0)
        {
            Logger::Log("Couldn't get serial port file descriptor. (Error no: "+to_string(errno)+")",
                        __FILE__, __LINE__,
                        Logger::LOG_TYPE::ERROR);

            this->last_err_code = errno;
            return;
        }

        struct termios config;
        memset(&config, 0, sizeof config);

        Logger::Log("Setting existing serial port configuration.",
                    __FILE__, __LINE__);
        if(tcgetattr(this->serial_port, &config) != 0)
        {
            Logger::Log("Couldn't set existing serial port configuration. (Error no: "+to_string(errno)+")",
                        __FILE__, __LINE__,
                        Logger::LOG_TYPE::ERROR);

            this->last_err_code = errno;
            return;
        }

        //configuration
        config.c_cflag &= ~PARENB;                 //clear parity bit, disabling parity
        config.c_cflag &= ~CSTOPB;                 //clear stop field, only one stop bit will be used
        config.c_cflag |=  CS8;                    //8 bits per byte
        config.c_cflag &= ~CRTSCTS;                //disable RTS/CTS hardware flow control
        config.c_cflag |=  CREAD | CLOCAL;         //turn on READ & ignore ctrl lines

        config.c_lflag &= ~ICANON;
        config.c_lflag &= ~ECHO;                   //disable echo
        config.c_lflag &= ~ECHOE;                  //disable erasure
        config.c_lflag &= ~ECHONL;                 //disable new-line echo
        config.c_lflag &= ~ISIG;                   //disable interpretation of INTR, QUIT, SUSP

        config.c_iflag &= ~(IXON | IXOFF | IXANY); //turn off s/w flow ctrl
        config.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); //disable any special handling of recieved bytes

        config.c_oflag &= ~OPOST;                  //prevent special interpretation of output byes (e.g. newline chars)
        config.c_oflag &= ~ONLCR;                  //prevent conversion of newline to carriage return/line feed

        config.c_cc[VTIME] = 10;                   //wait for up to 1s (10 deciseconds)
        config.c_cc[VMIN]  = 0;

        cfsetispeed(&config, this->baudrate);      //set in  baudrate to X
        cfsetospeed(&config, this->baudrate);      //set out baudrate to X

        Logger::Log("Configuring serial port.",
                    __FILE__, __LINE__);
        if(tcsetattr(this->serial_port, TCSANOW, &config) != 0)
        {
            Logger::Log("Couldn't configure serial port. (Error no: "+to_string(errno)+")",
                        __FILE__, __LINE__,
                        Logger::LOG_TYPE::ERROR);
            this->last_err_code = errno;
            return;
        }

        Logger::Log("Creating buffer with size "+to_string(this->buffer_size)+".",
                    __FILE__, __LINE__);
        this->buffer = new char[this->buffer_size];
        memset(this->buffer, '\0', this->buffer_size);

        this->status_code = 1;

        Logger::Log("Serial port successfully open.",
                    __FILE__, __LINE__);
    }

    void SerialPort::close()
    {
        if(this->status_code == 1) ::close(this->serial_port);
        else
        {

            Logger::Log("Cannot close the serial port. Status code is not 1.",
                        __FILE__, __LINE__,
                        Logger::LOG_TYPE::ERROR);
            return;
        }
    }

    void SerialPort::write(std::string const& data)
    {
        if(this->status_code == 0)
        {
            Logger::Log("Please open the port first before attempting to write into it.",
                        __FILE__, __LINE__,
                        Logger::LOG_TYPE::WARNING);
            return;
        }

        ::write(this->serial_port, data.c_str(), data.length());
    }

    std::string SerialPort::read()
    {
        if(this->status_code == 0)
        {
            Logger::Log("Please open the port first before attempting to read from it.",
                        __FILE__, __LINE__,
                        Logger::LOG_TYPE::WARNING);
            return "";
        }

        int bytes = ::read(this->serial_port, this->buffer, this->buffer_size);

        if(bytes < 0)
        {
            Logger::Log("An error occured while reading from serial port. (Recieved bytes: "+to_string(bytes)+")",
                        __FILE__, __LINE__,
                        Logger::LOG_TYPE::ERROR);
            return "";
        }

        return std::string(this->buffer, 0, bytes);
    }
}

