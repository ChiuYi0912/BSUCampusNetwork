#include <Windows.h>
#include <wlanapi.h>
#include <wininet.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Wlanapi.lib")

//获取当前连接的 WiFi
std::string GetCurrentWifi()
{
    HANDLE wlanHandle{};
    DWORD version{};

    DWORD result = WlanOpenHandle(
        2,
        nullptr,
        &version,
        &wlanHandle);

    if (result != ERROR_SUCCESS)
        return "";

    PWLAN_INTERFACE_INFO_LIST interfaces{};

    result = WlanEnumInterfaces(
        wlanHandle,
        nullptr,
        &interfaces);

    if (result != ERROR_SUCCESS)
    {
        WlanCloseHandle(wlanHandle, nullptr);
        return "";
    }

    for (DWORD i = 0; i < interfaces->dwNumberOfItems; i++)
    {
        auto &interfaceInfo =
            interfaces->InterfaceInfo[i];

        if (interfaceInfo.isState != wlan_interface_state_connected)
            continue;

        PWLAN_CONNECTION_ATTRIBUTES connection{};

        DWORD size{};

        result = WlanQueryInterface(
            wlanHandle,
            &interfaceInfo.InterfaceGuid,
            wlan_intf_opcode_current_connection,
            nullptr,
            &size,
            reinterpret_cast<PVOID *>(&connection),
            nullptr);

        if (result == ERROR_SUCCESS)
        {
            auto &ssid =
                connection->wlanAssociationAttributes.dot11Ssid;

            std::string name(
                reinterpret_cast<char *>(ssid.ucSSID),
                ssid.uSSIDLength);

            WlanFreeMemory(connection);
            WlanFreeMemory(interfaces);
            WlanCloseHandle(wlanHandle, nullptr);

            return name;
        }
    }

    WlanFreeMemory(interfaces);
    WlanCloseHandle(wlanHandle, nullptr);

    return "";
}

//连接BSU
bool ConnectToBSU()
{
    HANDLE h = nullptr;
    DWORD version = 0;

    if (WlanOpenHandle(2, nullptr, &version, &h) != ERROR_SUCCESS)
        return false;

    PWLAN_INTERFACE_INFO_LIST list = nullptr;

    if (WlanEnumInterfaces(h, nullptr, &list) != ERROR_SUCCESS ||
        list->dwNumberOfItems == 0)
    {
        WlanCloseHandle(h, nullptr);
        return false;
    }

    auto &wifi = list->InterfaceInfo[0];

    WLAN_CONNECTION_PARAMETERS p{};
    p.wlanConnectionMode = wlan_connection_mode_profile;
    p.strProfile = L"BSU";
    p.dot11BssType = dot11_BSS_type_any;

    if (WlanConnect(h, &wifi.InterfaceGuid, &p, nullptr) != ERROR_SUCCESS)
    {
        WlanFreeMemory(list);
        WlanCloseHandle(h, nullptr);
        return false;
    }

    //最多10秒
    for (int i = 0; i < 20; ++i)
    {
        WLAN_INTERFACE_STATE *state = nullptr;
        DWORD size = 0;

        if (WlanQueryInterface(
                h,
                &wifi.InterfaceGuid,
                wlan_intf_opcode_interface_state,
                nullptr,
                &size,
                reinterpret_cast<PVOID *>(&state),
                nullptr) == ERROR_SUCCESS)
        {
            bool connected =
                *state == wlan_interface_state_connected;

            WlanFreeMemory(state);

            if (connected)
            {
                WlanFreeMemory(list);
                WlanCloseHandle(h, nullptr);
                return true;
            }
        }

        Sleep(500);
    }

    WlanFreeMemory(list);
    WlanCloseHandle(h, nullptr);

    return false;
}

//检查网络是否可用
bool CheckCampusNetwork()
{
    return InternetGetConnectedState(nullptr, 0);
}

//登录校园网
bool LoginCampusNetwork()
{


    return false;
}

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

    //检查校园网
    if (CheckCampusNetwork())
    {
        return 0;
    }

    //登录
    if (!LoginCampusNetwork())
    {

        return 1;
    }

    return 0;
}