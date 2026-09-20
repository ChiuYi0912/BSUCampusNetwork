#pragma once

#include <Windows.h>

#include <string>

std::string HttpGet(
    const std::wstring &host,
    const std::wstring &path,
    DWORD &statusCode);
