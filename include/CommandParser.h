#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>

class CommandParser
{
public:
    using CommandHandler = std::function<void(const std::vector<std::string> &)>;

    void registerCommand(const std::string &name, CommandHandler handler);
    bool parseAndExecute(const std::string &input);
    void setErrorHandler(CommandHandler handler);

private:
    std::unordered_map<std::string, CommandHandler> commands_;
    CommandHandler errorHandler_;
};

#endif // COMMANDPARSER_H
