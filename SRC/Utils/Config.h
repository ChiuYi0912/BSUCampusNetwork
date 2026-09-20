#pragma once

#include <string>

struct Config
{
    std::string username;
    std::string password;
};

Config ReadConfig();
