#ifndef SERIALINTERFACE_H
#define SERIALINTERFACE_H
#include <vector>
#include <string>

class SerialInterface
{
public:
    SerialInterface();
    ~SerialInterface();

    bool open(const std::string &port);
    void close();
    bool sendData(const std::vector<unsigned char> &data);
    bool isOpen() const;

private:
    class SerialInterfaceImpl;
    SerialInterfaceImpl *impl_;
};

#endif // SERIALINTERFACE_H
