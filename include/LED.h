#ifndef LED_H
#define LED_H
#include <vector>
#include <random>

class LED
{
public:
    LED(int id, float peakWavelength, float maxRadiation);

    unsigned char getIntensity() const;
    void setIntensity(unsigned char value);

    unsigned char getMaxIntensity() const;
    void setMaxIntensity(unsigned char value);

    void randomizeIntensity();

    int getId() const;
    float getPeakWavelength() const;
    float getMaxRadiation() const;

    void lock();
    void unlock();
    bool isLocked() const;

private:
    int id_;
    float peakWavelength_;
    float maxRadiation_;
    unsigned char intensity_;
    unsigned char maxIntensity_ = 255;
    bool locked_ = false;
};

#endif // LED_H
