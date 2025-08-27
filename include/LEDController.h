#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "LED.h"

class LEDController
{
public:
    explicit LEDController(size_t count = 30);

    LED &getById(int id);
    const LED &getById(int id) const;

    LED &getByPeak(float peak);
    const LED &getByPeak(float peak) const;

    void setPortName(const std::string &portName);
    std::string getPortName() const;

    void randomizeAll();
    std::vector<unsigned char> getIntensityData() const;

    bool saveMaxIntensities(const std::string &filename) const;
    bool loadMaxIntensities(const std::string &filename);

private:
    std::vector<LED> leds_;
    std::string port_name_;
};

#endif // LEDCONTROLLER_H
