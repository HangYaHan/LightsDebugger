#ifndef CLIAPP_H
#define CLIAPP_H
#include "LEDController.h"
#include "SerialInterface.h"
#include "CommandParser.h"
#include <string>
#include <vector>
#include <map>
#include <functional>

class CLIApp
{
public:
    CLIApp();
    void run();

private:
    void setupCommands();
    void handleEmpty(const std::vector<std::string> &args);
    void handleSetCom(const std::vector<std::string> &args);
    void handleLS(const std::vector<std::string> &args);
    void handleSet(const std::vector<std::string> &args);
    void handleSetA(const std::vector<std::string> &args);
    void handleSetMA(const std::vector<std::string> &args);
    void handleSetM(const std::vector<std::string> &args);
    void handleRandom(const std::vector<std::string> &args);
    void handleSend(const std::vector<std::string> &args);
    void handleDo(const std::vector<std::string> &args);
    void handleSave(const std::vector<std::string> &args);
    void handleLoad(const std::vector<std::string> &args);
    void handleHelp(const std::vector<std::string> &args);
    void handleError(const std::vector<std::string> &args);
    void handleClear(const std::vector<std::string> &args);
    void handleLock(const std::vector<std::string> &args);
    void handleUnlock(const std::vector<std::string> &args);

    LEDController controller_;
    SerialInterface serial_;
    CommandParser parser_;
    std::string lastUsedPort_;
    std::string configFile_ = "led_config.cfg";
    std::map<std::string, std::function<void(const std::vector<std::string> &)>> commands;
};

#endif // CLIAPP_H
