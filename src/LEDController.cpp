#include "LEDController.h"
#include <string>

LEDController::LEDController(size_t count)
{
    // Each element: {id, peakWavelength, maxRadiation}
    static const struct
    {
        int id;
        float peak;
        float maxRad;
    } led_params[] = {
        {1, 660, 65000},
        {2, 0, 0}, // infrared
        {3, 750, 32500},
        {4, 490, 27500},
        {5, 405, 35000},
        {6, 645, 65000},
        {7, 590, 42500},
        {8, 505, 55000},
        {9, 545, 13000},
        {10, 0, 0}, // infrared
        {11, 0, 0}, // infrared
        {12, 0, 0}, // infrared
        {13, 770, 30000},
        {14, 570, 8500},
        {15, 870, 10000},
        {16, 0, 0}, // infrared
        {17, 525, 37500},
        {18, 800, 21000},
        {19, 0, 0}, // infrared
        {20, 430, 50000},
        {21, 830, 15000},
        {22, 680, 50000},
        {23, 450, 55000},
        {24, 610, 65000},
        {25, 900, 3000},
        {26, 0, 0}, // infrared
        {27, 0, 0}, // infrared
        {28, -1, -1}, // White LED, maxRadiation not defined
        {29, 0, 0}, // infrared
        {30, 625, 65000}};

    leds_.reserve(30);
    for (const auto &param : led_params)
    {
        leds_.emplace_back(param.id, param.peak, param.maxRad);
    }
}

LED &LEDController::getById(int id)
{
    for (auto &led : leds_)
    {
        if (led.getId() == id)
        {
            return led;
        }
    }
    throw std::out_of_range("LED id not found");
}

const LED &LEDController::getById(int id) const
{
    for (const auto &led : leds_)
    {
        if (led.getId() == id)
        {
            return led;
        }
    }
    throw std::out_of_range("LED is not found");
}

LED &LEDController::getByPeak(float peak)
{
    for (auto &led : leds_)
    {
        if (led.getPeakWavelength() == peak)
        {
            return led;
        }
    }
    throw std::out_of_range("LED with the specified peak wavelength is not found");
}

const LED &LEDController::getByPeak(float peak) const
{
    for (const auto &led : leds_)
    {
        if (led.getPeakWavelength() == peak)
        {
            return led;
        }
    }
    throw std::out_of_range("LED with the specified peak wavelength is not found");
}

void LEDController::randomizeAll()
{
    // Randomize the intensity of all LEDs (skip invalid LEDs)
    for (auto &led : leds_)
    {
        if (led.getPeakWavelength() != 0)
        {
            led.randomizeIntensity();
        }
    }
}

std::vector<unsigned char> LEDController::getIntensityData() const
{
    // Get intensity data for all LEDs (invalid LEDs have intensity 0)
    std::vector<unsigned char> data;
    data.reserve(leds_.size());
    for (const auto &led : leds_)
    {
        data.push_back(led.getIntensity());
    }
    return data;
}

bool LEDController::saveMaxIntensities(const std::string &filename) const
{
    std::ofstream ofs(filename, std::ios::out | std::ios::trunc);
    if (!ofs.is_open())
        return false;
    for (const auto &led : leds_)
    {
        ofs << led.getId() << " " << static_cast<int>(led.getMaxIntensity()) << "\n";
    }
    return true;
}

bool LEDController::loadMaxIntensities(const std::string &filename)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open())
        return false;
    int id, maxIntensity;
    while (ifs >> id >> maxIntensity)
    {
        try
        {
            getById(id).setMaxIntensity(static_cast<unsigned char>(maxIntensity));
        }
        catch (...)
        {
            // Ignore unknown LED IDs
            std::cerr << "Warning: LED ID " << id << " not found. Skipping.\n";
        }
    }
    return true;
}