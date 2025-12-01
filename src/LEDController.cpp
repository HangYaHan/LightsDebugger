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
        {1, 405, 35000},
        {2, 430, 50000},
        {3, 450, 55000},
        {4, 490, 27500},
        {5, 505, 55000},
        {6, 525, 37500},
        {7, 545, 13000},
        {8, 570, 8500},
        {9, 590, 13000},
        {10, 610, 65000},
        {11, 625, 65000},
        {12, 645, 65000},
        {13, 660, 65000},
        {14, 680, 50000},
        {15, 750, 32500},
        {16, 770, 30000},
        {17, 800, 21000},
        {18, 870, 0},
        {19, 970, 0},
        {20, 1050, 0},
        {21, 1200, 0},
        {22, 1300, 0},
        {23, 1450, 0},
        {24, 1550, 0},
        {25, 1600, 0},
        {26, 0, 0},
        {27, 1, 0},  // RED LED
        {28, 2, 0},  // GREEN LED
        {29, 3, 0},  // BLUE LED
        {30, -1, -1} // white LED
    };

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