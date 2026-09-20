#include "Config.h"

#include <Windows.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace
{
    std::string GetJsonString(
        const std::string &json,
        const std::string &key)
    {
        const std::string search = "\"" + key + "\"";

        size_t pos = json.find(search);
        if (pos == std::string::npos)
            return "";

        pos = json.find(':', pos);
        if (pos == std::string::npos)
            return "";

        pos = json.find('"', pos);
        if (pos == std::string::npos)
            return "";

        const size_t begin = pos + 1;

        const size_t end = json.find('"', begin);
        if (end == std::string::npos)
            return "";

        return json.substr(begin, end - begin);
    }
}

Config ReadConfig()
{
    wchar_t modulePath[MAX_PATH]{};
    GetModuleFileNameW(nullptr, modulePath, MAX_PATH);

    const std::filesystem::path exeDir =
        std::filesystem::path(modulePath).parent_path();

    const std::filesystem::path candidates[] = {
        exeDir / L"config.json",
        L"config.json"
    };

    for (const auto &file : candidates)
    {
        std::ifstream in(file);
        if (!in.is_open())
            continue;

        std::stringstream ss;
        ss << in.rdbuf();
        const std::string json = ss.str();

        Config config;
        config.username = GetJsonString(json, "username");
        config.password = GetJsonString(json, "password");
        return config;
    }

    return Config{};
}
