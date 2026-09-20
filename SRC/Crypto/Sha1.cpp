#include "Sha1.h"

#include <Windows.h>
#include <bcrypt.h>

#include <string>
#include <vector>

#pragma comment(lib, "bcrypt.lib")

std::string Sha1(
    const std::string &data)
{
    BCRYPT_ALG_HANDLE algorithm{};

    NTSTATUS status = BCryptOpenAlgorithmProvider(
        &algorithm,
        BCRYPT_SHA1_ALGORITHM,
        nullptr,
        0);

    if (status != 0)
        return "";

    DWORD objectSize{};
    DWORD resultSize{};

    status = BCryptGetProperty(
        algorithm,
        BCRYPT_OBJECT_LENGTH,
        reinterpret_cast<PUCHAR>(&objectSize),
        sizeof(objectSize),
        &resultSize,
        0);

    if (status != 0)
    {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return "";
    }

    std::vector<UCHAR> object(objectSize);

    BCRYPT_HASH_HANDLE hash{};

    status = BCryptCreateHash(
        algorithm,
        &hash,
        object.data(),
        objectSize,
        nullptr,
        0,
        0);

    if (status != 0)
    {
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return "";
    }

    status = BCryptHashData(
        hash,
        reinterpret_cast<PUCHAR>(
            const_cast<char *>(data.data())),
        static_cast<ULONG>(data.size()),
        0);

    if (status != 0)
    {
        BCryptDestroyHash(hash);
        BCryptCloseAlgorithmProvider(algorithm, 0);
        return "";
    }

    UCHAR digest[20]{};

    status = BCryptFinishHash(
        hash,
        digest,
        sizeof(digest),
        0);

    BCryptDestroyHash(hash);
    BCryptCloseAlgorithmProvider(algorithm, 0);

    if (status != 0)
        return "";

    static constexpr char hex[] =
        "0123456789abcdef";

    std::string result;

    for (UCHAR byte : digest)
    {
        result += hex[byte >> 4];
        result += hex[byte & 0x0F];
    }

    return result;
}
