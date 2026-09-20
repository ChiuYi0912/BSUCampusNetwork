#include "Url.h"

#include <string>

std::string UrlEncode(
    const std::string &input)
{
    static constexpr char hex[] =
        "0123456789ABCDEF";

    std::string result;

    for (unsigned char c : input)
    {
        if (
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' ||
            c == '_' ||
            c == '.' ||
            c == '~')
        {
            result += static_cast<char>(c);
        }
        else
        {
            result += '%';
            result += hex[c >> 4];
            result += hex[c & 0x0F];
        }
    }

    return result;
}
