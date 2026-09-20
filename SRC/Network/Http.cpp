#include "Http.h"

#include <winhttp.h>

#include <string>
#include <vector>

#pragma comment(lib, "winhttp.lib")

std::string HttpGet(
    const std::wstring &host,
    const std::wstring &path,
    DWORD &statusCode)
{
    statusCode = 0;

    HINTERNET session = WinHttpOpen(
        L"BSUCampusNetworkClient/1.0",
        WINHTTP_ACCESS_TYPE_NO_PROXY,
        nullptr,
        nullptr,
        0);

    if (!session)
        return "";

    HINTERNET connection = WinHttpConnect(
        session,
        host.c_str(),
        INTERNET_DEFAULT_HTTP_PORT,
        0);

    if (!connection)
    {
        WinHttpCloseHandle(session);
        return "";
    }

    HINTERNET request = WinHttpOpenRequest(
        connection,
        L"GET",
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        0);

    if (!request)
    {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return "";
    }

    BOOL result = WinHttpSendRequest(
        request,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0);

    if (!result)
    {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return "";
    }

    result = WinHttpReceiveResponse(
        request,
        nullptr);

    if (!result)
    {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return "";
    }

    DWORD statusSize = sizeof(statusCode);

    WinHttpQueryHeaders(
        request,
        WINHTTP_QUERY_STATUS_CODE |
            WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &statusCode,
        &statusSize,
        WINHTTP_NO_HEADER_INDEX);

    std::string response;

    while (true)
    {
        DWORD available{};

        if (!WinHttpQueryDataAvailable(
                request,
                &available))
        {
            break;
        }

        if (available == 0)
            break;

        std::vector<char> buffer(
            available + 1);

        DWORD read{};

        if (!WinHttpReadData(
                request,
                buffer.data(),
                available,
                &read))
        {
            break;
        }

        response.append(
            buffer.data(),
            read);
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);

    return response;
}
