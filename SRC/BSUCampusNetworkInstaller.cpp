#include <Windows.h>
#include <string>

int main()
{
    wchar_t path[MAX_PATH]{};

    GetModuleFileNameW(nullptr, path, MAX_PATH);

    std::wstring programPath = path;

    programPath = programPath.substr(
        0,
        programPath.find_last_of(L"\\/") + 1);

    programPath += L"BSUCampusNetworkClient.exe";

    HKEY key{};

    RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_SET_VALUE,
        &key);

    RegSetValueExW(
        key,
        L"BSUCampusNetworkClient",
        0,
        REG_SZ,
        reinterpret_cast<const BYTE *>(programPath.c_str()),
        static_cast<DWORD>(
            (programPath.size() + 1) * sizeof(wchar_t)));

    RegCloseKey(key);

    return 0;
}