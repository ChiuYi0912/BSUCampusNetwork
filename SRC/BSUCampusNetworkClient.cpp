#include "Network/Wifi.h"
#include "SRun/SRun.h"

#include <cstdlib>
#include <iostream>
#include <string>

int main()
{
    std::cout << "BSU Campus Network Client" << std::endl;

    std::string wifi = GetCurrentWifi();

    if (wifi != "BSU")
    {
        if (!ConnectToBSU())
        {
            return 1;
        }
    }

    // 检查校园网
    // if (CheckCampusNetwork())
    // {
    //     return 0;
    // }

    // 登录
    if (!LoginCampusNetwork())
    {
        system("ipconfig /flushdns > nul 2>&1");
        return 1;
    }

    return 0;
}
