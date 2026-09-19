#include <Windows.h>

int main()
{
    HKEY key{};

    RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_SET_VALUE,
        &key);

    RegDeleteValueW(
        key,
        L"BSUCampusNetworkClient");

    RegCloseKey(key);

    return 0;
}