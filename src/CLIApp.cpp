#include "CLIApp.h"
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <iomanip>

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

        // 在回放模式下，仅支持空输入(回放下一帧)和 replay -e
        if (isReplaying_)
        {
            if (args.empty())
            {
                handleEmpty(args);
                continue;
            }
            if (!args.empty() && args[0] == "replay")
            {
                handleReplay(args);
                continue;
            }
            std::cout << "[Warn] In replay mode: only (empty) or 'replay -e' supported.\n";
            continue;
        }

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
    commands["lock"] = [this](const std::vector<std::string> &args)
    { handleLock(args); };
    commands["unlock"] = [this](const std::vector<std::string> &args)
    { handleUnlock(args); };
    commands["record"] = [this](const std::vector<std::string> &args)
    { handleRecord(args); };
    commands["replay"] = [this](const std::vector<std::string> &args)
    { handleReplay(args); };
}

void CLIApp::handleEmpty(const std::vector<std::string> &args)
{
    // 空命令：在回放模式下发送下一帧；否则随机并发送
    if (isReplaying_)
    {
        if (!serial_.isOpen())
        {
            std::cout << "[Error] Serial port not open. Use setcom to set port.\n";
            return;
        }
        std::vector<unsigned char> packet;
        if (!readNextReplayPacket(packet))
        {
            std::cout << "[Info] Replay finished or no more data. Use 'replay -e' to exit.\n";
            return;
        }

        std::cout << "[Debug] Send packet: ";
        for (auto b : packet)
            std::cout << std::hex << std::uppercase << (int)b << " ";
        std::cout << std::dec << std::endl;

        if (serial_.sendData(packet))
        {
            maybeRecordPacket(packet);
            std::cout << "[Info] Replay frame sent.\n";
        }
        else
        {
            std::cout << "[Error] Failed to send data to serial port.\n";
        }
        return;
    }

    // 非回放：随机生成并发送
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
        maybeRecordPacket(packet);
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
    std::cout << "ID\tPeak\tMaxRad\tIntensity\tMaxIntensity\tLocked\n";
    for (int i = 1; i <= 30; ++i)
    {
        try
        {
            const auto &led = controller_.getById(i);
            std::cout << led.getId() << "\t"
                      << led.getPeakWavelength() << "\t"
                      << led.getMaxRadiation() << "\t"
                      << (int)led.getIntensity() << "\t\t"
                      << (int)led.getMaxIntensity() << "\t\t"
                      << (led.isLocked() ? "Yes" : "No") << "\n";
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
            if (controller_.getById(id).isLocked())
            {
                std::cout << "[Warning] LED #" << id << " is locked. Intensity not changed.\n";
                return;
            }
            else
            {
                controller_.getById(id).setIntensity((unsigned char)value);
                std::cout << "[Info] LED #" << id << " intensity set to " << value << "\n";
            }
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
            if (controller_.getByPeak(peak).isLocked())
            {
                std::cout << "[Warning] LED with peak " << peak << " is locked. Intensity not changed.\n";
                return;
            }
            else
            {
                controller_.getByPeak(peak).setIntensity((unsigned char)value);
                std::cout << "[Info] LED with peak " << peak << " intensity set to " << value << "\n";
            }
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
            if (controller_.getById(i).isLocked())
            {
                std::cout << "[Warning] LED #" << i << " is locked. Intensity not changed.\n";
                continue;
            }
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
        maybeRecordPacket(packet);
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
        else
        {
            maybeRecordPacket(packet);
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
                 "  record -s [f]   : Start recording sent packets to file f (default record.txt)\n"
                 "  record -e       : Stop recording\n"
                 "  replay -s [f]   : Start replay from file f (default record.txt)\n"
                 "  replay -e       : Stop replay mode\n"
                 "  lock l<x>       : Lock LED by id (prevent changes)\n"
                 "  lock <peak>     : Lock LED by peak (prevent changes)\n"
                 "  lock all        : Lock all LEDs (prevent changes)\n"
                 "  unlock l<x>     : Unlock LED by id (allow changes)\n"
                 "  unlock <peak>   : Unlock LED by peak (allow changes)\n"
                 "  unlock all      : Unlock all LEDs (allow changes)\n"
                 "  cls             : Clear all LED intensities\n";
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

void CLIApp::handleLock(const std::vector<std::string> &args)
{
    // lock all
    if (args.size() == 2 && args[1] == "all")
    {
        for (int i = 1; i <= 30; ++i)
        {
            try
            {
                controller_.getById(i).lock();
            }
            catch (...)
            {
                // 忽略无效 id
            }
        }
        std::cout << "[Info] All LEDs locked.\n";
        return;
    }

    // lock l<x> 或 lock <peak>
    if (args.size() == 2 && args[1].size() > 1 && args[1][0] == 'l')
    {
        // lock l<x>
        int id = std::stoi(args[1].substr(1));
        try
        {
            controller_.getById(id).lock();
            std::cout << "[Info] LED #" << id << " locked.\n";
        }
        catch (...)
        {
            std::cout << "[Error] Invalid LED id.\n";
        }
    }
    else if (args.size() == 2)
    {
        // lock <peak>
        float peak = std::stof(args[1]);
        try
        {
            controller_.getByPeak(peak).lock();
            std::cout << "[Info] LED with peak " << peak << " locked.\n";
        }
        catch (...)
        {
            std::cout << "[Error] Invalid peak value.\n";
        }
    }
    else
    {
        std::cout << "[Usage] lock l<x>  or  lock <peak>\n";
    }
}

void CLIApp::handleUnlock(const std::vector<std::string> &args)
{
    // unlock all
    if (args.size() == 2 && args[1] == "all")
    {
        for (int i = 1; i <= 30; ++i)
        {
            try
            {
                controller_.getById(i).unlock();
            }
            catch (...)
            {
                // 忽略无效 id
            }
        }
        std::cout << "[Info] All LEDs unlocked.\n";
        return;
    }

    // unlock l<x> 或 unlock <peak>
    if (args.size() == 2 && args[1].size() > 1 && args[1][0] == 'l')
    {
        // unlock l<x>
        int id = std::stoi(args[1].substr(1));
        try
        {
            controller_.getById(id).unlock();
            std::cout << "[Info] LED #" << id << " unlocked.\n";
        }
        catch (...)
        {
            std::cout << "[Error] Invalid LED id.\n";
        }
    }
    else if (args.size() == 2)
    {
        // unlock <peak>
        float peak = std::stof(args[1]);
        try
        {
            controller_.getByPeak(peak).unlock();
            std::cout << "[Info] LED with peak " << peak << " unlocked.\n";
        }
        catch (...)
        {
            std::cout << "[Error] Invalid peak value.\n";
        }
    }
    else
    {
        std::cout << "[Usage] unlock l<x>  or  unlock <peak>\n";
    }
}

void CLIApp::handleRecord(const std::vector<std::string> &args)
{
    // record -s [file]  或  record -e
    if (args.size() < 2)
    {
        std::cout << "[Error] Usage: record -s [filename]  |  record -e\n";
        return;
    }
    if (args[1] == "-s")
    {
        std::string path = (args.size() >= 3) ? args[2] : std::string("record.txt");
        if (recordFile_.is_open())
        {
            recordFile_.close();
        }
        recordFile_.open(path, std::ios::out | std::ios::trunc);
        if (!recordFile_.is_open())
        {
            std::cout << "[Error] Failed to open record file: " << path << "\n";
            return;
        }
        recordFilePath_ = path;
        isRecording_ = true;
        std::cout << "[Info] Recording started to '" << recordFilePath_ << "'.\n";
    }
    else if (args[1] == "-e")
    {
        if (!isRecording_)
        {
            std::cout << "[Info] Recording has not been started. Use 'record -s [file]'.\n";
            return;
        }
        if (recordFile_.is_open())
            recordFile_.close();
        isRecording_ = false;
        std::cout << "[Info] Recording stopped.\n";
    }
    else
    {
        std::cout << "[Error] Usage: record -s [filename]  |  record -e\n";
    }
}

void CLIApp::handleReplay(const std::vector<std::string> &args)
{
    // replay -s [file]  或  replay -e
    if (args.size() < 2)
    {
        std::cout << "[Error] Usage: replay -s [filename]  |  replay -e\n";
        return;
    }
    if (args[1] == "-s")
    {
        std::string path = (args.size() >= 3) ? args[2] : std::string("record.txt");
        if (replayFile_.is_open())
            replayFile_.close();
        replayFile_.open(path);
        if (!replayFile_.is_open())
        {
            std::cout << "[Error] Failed to open replay file: " << path << "\n";
            return;
        }
        replayFilePath_ = path;
        isReplaying_ = true;
        std::cout << "[Info] Replay started from '" << replayFilePath_ << "'. Press Enter to send next frame.\n";
    }
    else if (args[1] == "-e")
    {
        if (!isReplaying_)
        {
            std::cout << "[Info] Replay has not been started. Use 'replay -s [file]'.\n";
            return;
        }
        if (replayFile_.is_open())
            replayFile_.close();
        isReplaying_ = false;
        std::cout << "[Info] Replay stopped.\n";
    }
    else
    {
        std::cout << "[Error] Usage: replay -s [filename]  |  replay -e\n";
    }
}

void CLIApp::maybeRecordPacket(const std::vector<unsigned char> &packet)
{
    if (!isRecording_ || !recordFile_.is_open())
        return;
    // 按行记录：32个两位十六进制数（大写，零填充）
    for (size_t i = 0; i < packet.size(); ++i)
    {
        if (i)
            recordFile_ << ' ';
        recordFile_ << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)packet[i];
    }
    recordFile_ << std::dec << "\n";
    recordFile_.flush();
}

bool CLIApp::readNextReplayPacket(std::vector<unsigned char> &packet)
{
    if (!replayFile_.is_open())
        return false;
    std::string line;
    if (!std::getline(replayFile_, line))
        return false;
    std::istringstream iss(line);
    std::string tok;
    std::vector<unsigned char> bytes;
    while (iss >> tok)
    {
        try
        {
            int v = std::stoi(tok, nullptr, 16);
            if (v < 0)
                v = 0;
            if (v > 255)
                v = 255;
            bytes.push_back(static_cast<unsigned char>(v));
        }
        catch (...)
        {
            // 非法行，标记失败
            bytes.clear();
            break;
        }
    }
    if (bytes.size() != 32)
    {
        std::cout << "[Error] Invalid replay line (expect 32 bytes).\n";
        return false;
    }
    packet = std::move(bytes);
    return true;
}
