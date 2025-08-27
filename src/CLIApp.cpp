#include "CLIApp.h"
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>

CLIApp::CLIApp()
{
    setupCommands();
}

void CLIApp::run()
{
    std::string line;
    while (true)
    {
        std::cout << "> ";
        if (!std::getline(std::cin, line))
            break;
        std::vector<std::string> args;
        std::istringstream iss(line);
        std::string arg;
        while (iss >> arg)
            args.push_back(arg);

        if (args.empty())
        {
            handleEmpty(args);
            continue;
        }

        auto it = commands.find(args[0]);
        if (it != commands.end())
        {
            it->second(args);
        }
        else
        {
            handleError(args);
        }
    }
}

void CLIApp::setupCommands()
{
    commands["setcom"] = [this](const std::vector<std::string> &args)
    { handleSetCom(args); };
    commands["ls"] = [this](const std::vector<std::string> &args)
    { handleLS(args); };
    commands["set"] = [this](const std::vector<std::string> &args)
    { handleSet(args); };
    commands["seta"] = [this](const std::vector<std::string> &args)
    { handleSetA(args); };
    commands["setma"] = [this](const std::vector<std::string> &args)
    { handleSetMA(args); };
    commands["setm"] = [this](const std::vector<std::string> &args)
    { handleSetM(args); };
    commands["random"] = [this](const std::vector<std::string> &args)
    { handleRandom(args); };
    commands["send"] = [this](const std::vector<std::string> &args)
    { handleSend(args); };
    commands["do"] = [this](const std::vector<std::string> &args)
    { handleDo(args); };
    commands["save"] = [this](const std::vector<std::string> &args)
    { handleSave(args); };
    commands["load"] = [this](const std::vector<std::string> &args)
    { handleLoad(args); };
    commands["help"] = [this](const std::vector<std::string> &args)
    { handleHelp(args); };
    commands["cls"] = [this](const std::vector<std::string> &args)
    { handleClear(args); };
}

void CLIApp::handleEmpty(const std::vector<std::string> &args)
{
    // 空命令：随机生成强度并发送到串口
    controller_.randomizeAll();
    auto data = controller_.getIntensityData();
    if (!serial_.isOpen())
    {
        std::cout << "[Error] Serial port not open. Use setcom to set port.\n";
        return;
    }
    std::vector<unsigned char> packet;
    packet.reserve(32);
    packet.push_back(0xDA);
    packet.push_back(0xAD);
    packet.insert(packet.end(), data.begin(), data.end());

    // 输出即将发送的数据
    std::cout << "[Debug] Send packet: ";
    for (auto b : packet)
        std::cout << std::hex << std::uppercase << (int)b << " ";
    std::cout << std::dec << std::endl;

    if (serial_.sendData(packet))
    {
        std::cout << "[Info] Random intensity generated and sent.\n";
    }
    else
    {
        std::cout << "[Error] Failed to send data to serial port.\n";
    }
}

void CLIApp::handleSetCom(const std::vector<std::string> &args)
{
    // setcom COMx
    if (args.size() != 2)
    {
        std::cout << "[Usage] setcom COMx\n";
        return;
    }
    if (serial_.open(args[1]))
    {
        lastUsedPort_ = args[1];
        std::cout << "[Info] Serial port set to " << args[1] << "\n";
    }
    else
    {
        std::cout << "[Error] Failed to open serial port " << args[1] << "\n";
    }
}

void CLIApp::handleLS(const std::vector<std::string> &args)
{
    // 输出30个灯的详细信息
    std::cout << "ID\tPeak\tMaxRad\tIntensity\tMaxIntensity\n";
    for (int i = 1; i <= 30; ++i)
    {
        try
        {
            const auto &led = controller_.getById(i);
            std::cout << led.getId() << "\t"
                      << led.getPeakWavelength() << "\t"
                      << led.getMaxRadiation() << "\t"
                      << (int)led.getIntensity() << "\t\t"
                      << (int)led.getMaxIntensity() << "\n";
        }
        catch (...)
        {
            std::cout << i << "\t[Invalid]\n";
        }
    }
}

void CLIApp::handleSet(const std::vector<std::string> &args)
{
    // set l<x> y 或 set x y
    if (args.size() == 3 && args[1].size() > 1 && args[1][0] == 'l')
    {
        // set l<x> y
        int id = std::stoi(args[1].substr(1));
        int value = std::stoi(args[2]);
        try
        {
            controller_.getById(id).setIntensity((unsigned char)value);
            std::cout << "[Info] LED #" << id << " intensity set to " << value << "\n";
        }
        catch (...)
        {
            std::cout << "[Error] Invalid LED id.\n";
        }
    }
    else if (args.size() == 3)
    {
        // set x y (x为峰位)
        float peak = std::stof(args[1]);
        int value = std::stoi(args[2]);
        try
        {
            controller_.getByPeak(peak).setIntensity((unsigned char)value);
            std::cout << "[Info] LED with peak " << peak << " intensity set to " << value << "\n";
        }
        catch (...)
        {
            std::cout << "[Error] Invalid peak value.\n";
        }
    }
    else
    {
        std::cout << "[Usage] set l<x> y  or  set <peak> <value>\n";
    }
}

void CLIApp::handleSetA(const std::vector<std::string> &args)
{
    // seta x
    if (args.size() != 2)
    {
        std::cout << "[Usage] seta <value>\n";
        return;
    }
    int value = std::stoi(args[1]);
    for (int i = 1; i <= 30; ++i)
    {
        try
        {
            controller_.getById(i).setIntensity((unsigned char)value);
        }
        catch (...)
        {
        }
    }
    std::cout << "[Info] All LEDs intensity set to " << value << "\n";
}

void CLIApp::handleSetMA(const std::vector<std::string> &args)
{
    // setma x
    if (args.size() != 2)
    {
        std::cout << "[Usage] setma <value>\n";
        return;
    }
    int value = std::stoi(args[1]);
    for (int i = 1; i <= 30; ++i)
    {
        try
        {
            controller_.getById(i).setMaxIntensity((unsigned char)value);
        }
        catch (...)
        {
        }
    }
    std::cout << "[Info] All LEDs max intensity set to " << value << "\n";
}

void CLIApp::handleSetM(const std::vector<std::string> &args)
{
    // setm l<x> y 或 setm x y
    if (args.size() == 3 && args[1].size() > 1 && args[1][0] == 'l')
    {
        // setm l<x> y
        int id = std::stoi(args[1].substr(1));
        int value = std::stoi(args[2]);
        try
        {
            controller_.getById(id).setMaxIntensity((unsigned char)value);
            std::cout << "[Info] LED #" << id << " max intensity set to " << value << "\n";
        }
        catch (...)
        {
            std::cout << "[Error] Invalid LED id.\n";
        }
    }
    else if (args.size() == 3)
    {
        // setm x y (x为峰位)
        float peak = std::stof(args[1]);
        int value = std::stoi(args[2]);
        try
        {
            controller_.getByPeak(peak).setMaxIntensity((unsigned char)value);
            std::cout << "[Info] LED with peak " << peak << " max intensity set to " << value << "\n";
        }
        catch (...)
        {
            std::cout << "[Error] Invalid peak value.\n";
        }
    }
    else
    {
        std::cout << "[Usage] setm l<x> y  or  setm <peak> <value>\n";
    }
}

void CLIApp::handleRandom(const std::vector<std::string> &args)
{
    // 随机生成强度
    controller_.randomizeAll();
    std::cout << "[Info] Random intensity generated.\n";
}

void CLIApp::handleSend(const std::vector<std::string> &args)
{
    // 发送当前强度数据到串口
    if (!serial_.isOpen())
    {
        std::cout << "[Error] Serial port not open. Use setcom to set port.\n";
        return;
    }
    auto data = controller_.getIntensityData();
    std::vector<unsigned char> packet;
    packet.reserve(32);
    packet.push_back(0xDA);
    packet.push_back(0xAD);
    packet.insert(packet.end(), data.begin(), data.end());

    // 输出即将发送的数据
    std::cout << "[Debug] Send packet: ";
    for (auto b : packet)
        std::cout << std::hex << std::uppercase << (int)b << " ";
    std::cout << std::dec << std::endl;

    if (serial_.sendData(packet))
    {
        std::cout << "[Info] Data sent to serial port.\n";
    }
    else
    {
        std::cout << "[Error] Failed to send data to serial port.\n";
    }
}

void CLIApp::handleDo(const std::vector<std::string> &args)
{
    // do X：random+send执行Y次，每次间隔1秒
    if (args.size() != 2)
    {
        std::cout << "[Usage] do <count>\n";
        return;
    }
    int count = std::stoi(args[1]);
    for (int i = 0; i < count; ++i)
    {
        controller_.randomizeAll();
        auto data = controller_.getIntensityData();
        std::vector<unsigned char> packet;
        packet.reserve(32);
        packet.push_back(0xDA);
        packet.push_back(0xAD);
        packet.insert(packet.end(), data.begin(), data.end());

        // 输出即将发送的数据
        std::cout << "[Debug] Send packet: ";
        for (auto b : packet)
            std::cout << std::hex << std::uppercase << (int)b << " ";
        std::cout << std::dec << std::endl;

        if (!serial_.isOpen() || !serial_.sendData(packet))
        {
            std::cout << "[Error] Serial port not open or send failed.\n";
            break;
        }
        std::cout << "[Info] [" << (i + 1) << "/" << count << "] Data sent.\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void CLIApp::handleSave(const std::vector<std::string> &args)
{
    // 保存强度上限到文件
    if (controller_.saveMaxIntensities(configFile_))
    {
        std::cout << "[Info] Max intensities saved to " << configFile_ << "\n";
    }
    else
    {
        std::cout << "[Error] Failed to save max intensities.\n";
    }
}

void CLIApp::handleLoad(const std::vector<std::string> &args)
{
    // 从文件读取强度上限
    if (controller_.loadMaxIntensities(configFile_))
    {
        std::cout << "[Info] Max intensities loaded from " << configFile_ << "\n";
    }
    else
    {
        std::cout << "[Error] Failed to load max intensities.\n";
    }
}

void CLIApp::handleHelp(const std::vector<std::string> &args)
{
    std::cout << "Available commands:\n"
                 "  (empty)         : Generate random intensities and send to COM port\n"
                 "  setcom COMx     : Set output serial port\n"
                 "  ls              : List all 30 LEDs info\n"
                 "  set l<x> y      : Set LED by id to intensity y\n"
                 "  set <peak> y    : Set LED by peak to intensity y\n"
                 "  seta x          : Set all LEDs intensity to x\n"
                 "  setma x         : Set all LEDs max intensity to x\n"
                 "  setm l<x> y     : Set LED by id max intensity to y\n"
                 "  setm <peak> y   : Set LED by peak max intensity to y\n"
                 "  random          : Generate random intensities\n"
                 "  send            : Send current intensities to serial port\n"
                 "  do X            : random+send X times, 1s interval\n"
                 "  save            : Save max intensities to file\n"
                 "  load            : Load max intensities from file\n"
                 "  help            : Show this help\n"
                 "  cls           : Clear all LED intensities\n";
}

void CLIApp::handleError(const std::vector<std::string> &args)
{
    std::cout << "[Error] Unknown or invalid command. Type 'help' for usage.\n";
}

void CLIApp::handleClear(const std::vector<std::string> &args)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}