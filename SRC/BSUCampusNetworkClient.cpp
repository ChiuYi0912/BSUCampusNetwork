#include <cstdlib>
#include <iostream>
#include <string>

#include <Windows.h>

#include "Network/Wifi.h"
#include "SRun/SRun.h"

constexpr const char *kBsuSsid = "BSU";

enum class ExitCode
{
    Success = 0,
    ConnectFailed = 1,
    LoginFailed = 2,
};

bool FlushDns()
{
    return system("ipconfig /flushdns > nul 2>&1") == 0;
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "BSU Campus Network Client\n";

    if (GetCurrentWifi() != kBsuSsid)
    {
        if (!ConnectToBSU())
        {
            std::cerr << "连接 BSU 失败\n";
            return static_cast<int>(ExitCode::ConnectFailed);
        }
    }

    FlushDns();

    // 登录
    if (!LoginCampusNetwork())
    {
        std::cerr << "登录失败 (可能是认证服务器ip变化,或者不需要认证)\n";
        FlushDns();

        std::cout << "按 Enter 键退出...\n";
        std::cin.get();

        return static_cast<int>(ExitCode::LoginFailed);
    }

    std::cout << "登录成功\n";
    return static_cast<int>(ExitCode::Success);
}
