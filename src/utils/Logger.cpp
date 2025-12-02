#include "Logger.h"
#include <iostream>
std::ofstream Logger::file_;

void Logger::init(const std::string& file) {
    file_.open(file, std::ios::app);
}

void Logger::LOG_INFO(const std::string& msg) {
    file_ << "[INFO] " << msg << std::endl;
    std::cout << "[INFO] " << msg << std::endl;
}
// Similar for others
