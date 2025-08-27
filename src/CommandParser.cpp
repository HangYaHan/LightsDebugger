#include "CommandParser.h"
#include <sstream>

void CommandParser::registerCommand(const std::string &name, CommandHandler handler)
{
    // 注册命令及其处理函数
    commands_[name] = handler;
}

bool CommandParser::parseAndExecute(const std::string &input)
{
    // 解析输入并执行对应命令
    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;
    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg)
    {
        args.push_back(arg);
    }
    auto it = commands_.find(cmd);
    if (it != commands_.end())
    {
        it->second(args);
        return true;
    }
    else if (errorHandler_)
    {
        errorHandler_({cmd});
    }
    return false;
}

void CommandParser::setErrorHandler(CommandHandler handler)
{
    // 设置错误处理函数
    errorHandler_ = handler;
}