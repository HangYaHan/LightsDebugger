#include "LED.h"

LED::LED(int id, float peakWavelength, float maxRadiation)
    : id_(id),
      peakWavelength_(peakWavelength),
      maxRadiation_(maxRadiation),
      intensity_(0),
      maxIntensity_(255)
{
    // if the light is infrared
    if (peakWavelength_ == 0)
    {
        intensity_ = 0;
        maxIntensity_ = 0;
    }
}

unsigned char LED::getIntensity() const
{
    return intensity_;
}

void LED::setIntensity(unsigned char value)
{
    intensity_ = value;
}

unsigned char LED::getMaxIntensity() const
{
    return maxIntensity_;
}

void LED::setMaxIntensity(unsigned char value)
{
    maxIntensity_ = value;
}

void LED::randomizeIntensity()
{
    // 生成随机强度
    intensity_ = rand() % (maxIntensity_ + 1);
}

int LED::getId() const
{
    return id_;
}

float LED::getPeakWavelength() const
{
    return peakWavelength_;
}

float LED::getMaxRadiation() const
{
    return maxRadiation_;
}