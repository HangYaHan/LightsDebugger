#include "SerialInterface.h"
#include <windows.h>
#include <string>

class SerialInterface::SerialInterfaceImpl
{
public:
    HANDLE hSerial = INVALID_HANDLE_VALUE;
};

SerialInterface::SerialInterface() : impl_(new SerialInterfaceImpl) {}

SerialInterface::~SerialInterface()
{
    close();
    delete impl_;
}

bool SerialInterface::open(const std::string &port)
{
    if (isOpen())
        close();
    std::string fullPort = "\\\\.\\" + port;
    impl_->hSerial = CreateFileA(
        fullPort.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);
    if (impl_->hSerial == INVALID_HANDLE_VALUE)
        return false;

    // 设置串口参数
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(impl_->hSerial, &dcb))
    {
        close();
        return false;
    }
    dcb.BaudRate = CBR_115200;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    if (!SetCommState(impl_->hSerial, &dcb))
    {
        close();
        return false;
    }

    // 设置超时
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(impl_->hSerial, &timeouts))
    {
        close();
        return false;
    }

    return true;
}

void SerialInterface::close()
{
    if (impl_ && impl_->hSerial != INVALID_HANDLE_VALUE)
    {
        CloseHandle(impl_->hSerial);
        impl_->hSerial = INVALID_HANDLE_VALUE;
    }
}

bool SerialInterface::sendData(const std::vector<unsigned char> &data)
{
    if (!isOpen() || data.empty())
        return false;
    DWORD bytesWritten = 0;
    BOOL ok = WriteFile(impl_->hSerial, data.data(), static_cast<DWORD>(data.size()), &bytesWritten, nullptr);
    return ok && bytesWritten == data.size();
}

bool SerialInterface::isOpen() const
{
    return impl_ && impl_->hSerial != INVALID_HANDLE_VALUE;
}