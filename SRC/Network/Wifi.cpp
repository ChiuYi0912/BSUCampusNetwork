#include "Wifi.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <ipifcons.h>
#include <Windows.h>
#include <wlanapi.h>
#include <wininet.h>

#include <string>
#include <vector>
#include <cstdint>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Wlanapi.lib")

struct WsaInit
{
    bool ok = false;
    WsaInit()
    {
        WSADATA wsaData;
        ok = (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
    }
    ~WsaInit()
    {
        if (ok)
        {
            WSACleanup();
        }
    }
};

// 获取当前连接的 WiFi
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

// 连接BSU
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

    // 最多10秒
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

// 检查网络是否可用
bool CheckCampusNetwork()
{
    return InternetGetConnectedState(nullptr, 0);
}

std::string GetWifiIPv4()
{
    static WsaInit wsa;
    if (!wsa.ok)
    {
        return "";
    }

    const ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;

    ULONG bufLen = 0;
    ULONG ret = GetAdaptersAddresses(AF_INET, flags, nullptr, nullptr, &bufLen);
    if (ret != ERROR_BUFFER_OVERFLOW)
    {
        return "";
    }

    std::vector<BYTE> buffer(bufLen);
    PIP_ADAPTER_ADDRESSES pAddrs =
        reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());

    ret = GetAdaptersAddresses(AF_INET, flags, nullptr, pAddrs, &bufLen);
    if (ret != NO_ERROR)
    {
        return "";
    }

    for (PIP_ADAPTER_ADDRESSES p = pAddrs; p != nullptr; p = p->Next)
    {
        // 只取无线网卡
        if (p->IfType != IF_TYPE_IEEE80211)
        {
            continue;
        }

        // 只取已连接网卡
        if (p->OperStatus != IfOperStatusUp)
        {
            continue;
        }

        for (PIP_ADAPTER_UNICAST_ADDRESS u = p->FirstUnicastAddress;
             u != nullptr; u = u->Next)
        {

            if (u->Address.lpSockaddr == nullptr)
            {
                continue;
            }

            if (u->Address.lpSockaddr->sa_family != AF_INET)
            {
                continue;
            }

            sockaddr_in *sin =
                reinterpret_cast<sockaddr_in *>(u->Address.lpSockaddr);

            char ip[INET_ADDRSTRLEN] = {0};
            if (inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip)) != nullptr)
            {
                std::string result(ip);
                if (!result.empty() && result != "0.0.0.0")
                {
                    return result;
                }
            }
        }
    }

    return "";
}
