#include "utils.h"
#include <iostream>

namespace Logger
{
    void info(const std::string &message)
    {
        std::cout << "[INFO] " << message << std::endl;
    }
    void warning(const std::string &message)
    {
        std::cout << "[WARN] " << message << std::endl;
    }
    void error(const std::string &message)
    {
        std::cerr << "[ERROR] " << message << std::endl;
    }
}
