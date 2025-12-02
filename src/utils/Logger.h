#pragma once
#include <string>
#include <fstream>

class Logger {
public:
    static void init(const std::string& file);
    static void LOG_INFO(const std::string& msg);
    static void LOG_WARN(const std::string& msg);
    static void LOG_ERROR(const std::string& msg);
    static void LOG_DEBUG(const std::string& msg);
private:
    static std::ofstream file_;
};
