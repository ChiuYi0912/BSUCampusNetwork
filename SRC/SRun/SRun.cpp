#include "SRun.h"

#include "../Network/Wifi.h"
#include "../Network/Http.h"
#include "../Crypto/Md5.h"
#include "../Crypto/Sha1.h"
#include "../Utils/Url.h"

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

static std::vector<uint32_t> XEncodeToBytes(
    const std::string& input,
    const std::string& key)
{
    std::vector<uint32_t> v;

    //JS:
    //
    // for (var i = 0; i < c; i += 4) {
    //     v[i >> 2] =
    //         a.charCodeAt(i)
    //         | a.charCodeAt(i + 1) << 8
    //         | a.charCodeAt(i + 2) << 16
    //         | a.charCodeAt(i + 3) << 24;
    // }

    for (size_t i = 0; i < input.size(); i += 4)
    {
        uint32_t value = 0;

        if (i < input.size())
        {
            value |= static_cast<uint32_t>(
                static_cast<unsigned char>(input[i]));
        }

        if (i + 1 < input.size())
        {
            value |= static_cast<uint32_t>(
                static_cast<unsigned char>(input[i + 1])) << 8;
        }

        if (i + 2 < input.size())
        {
            value |= static_cast<uint32_t>(
                static_cast<unsigned char>(input[i + 2])) << 16;
        }

        if (i + 3 < input.size())
        {
            value |= static_cast<uint32_t>(
                static_cast<unsigned char>(input[i + 3])) << 24;
        }

        v.push_back(value);
    }

    // JS:
    // if (b) v[v.length] = c;

    v.push_back(
        static_cast<uint32_t>(input.size()));


    // key
    std::vector<uint32_t> k;

    for (size_t i = 0; i < key.size(); i += 4)
    {
        uint32_t value = 0;

        if (i < key.size())
        {
            value |= static_cast<uint32_t>(
                static_cast<unsigned char>(key[i]));
        }

        if (i + 1 < key.size())
        {
            value |= static_cast<uint32_t>(
                static_cast<unsigned char>(key[i + 1])) << 8;
        }

        if (i + 2 < key.size())
        {
            value |= static_cast<uint32_t>(
                static_cast<unsigned char>(key[i + 2])) << 16;
        }

        if (i + 3 < key.size())
        {
            value |= static_cast<uint32_t>(
                static_cast<unsigned char>(key[i + 3])) << 24;
        }

        k.push_back(value);
    }

    while (k.size() < 4)
    {
        k.push_back(0);
    }


    // xEncode
    const size_t n = v.size() - 1;

    uint32_t z = v[n];
    uint32_t y = v[0];

    constexpr uint32_t delta =
        0x9E3779B9u;

    uint32_t d = 0;

    uint32_t q =
        static_cast<uint32_t>(
            6 + 52 / (n + 1));


    while (q-- > 0)
    {
        d += delta;

        uint32_t e =
            (d >> 2) & 3;


        for (size_t p = 0; p < n; ++p)
        {
            y = v[p + 1];

            // JS:
            //
            // m = z >>> 5 ^ y << 2;
            // m += (y >>> 3 ^ z << 4) ^ (d ^ y);
            // m += k[(p & 3) ^ e] ^ z;

            uint32_t m =
                (z >> 5) ^ (y << 2);

            m +=
                ((y >> 3) ^ (z << 4))
                ^ (d ^ y);

            m +=
                k[(p & 3) ^ e] ^ z;

            z =
                v[p] + m;

            v[p] =
                z;
        }


        // JS:
        //
        // y = v[0];
        // m = z >>> 5 ^ y << 2;
        // m += (y >>> 3 ^ z << 4) ^ (d ^ y);
        // m += k[(p & 3) ^ e] ^ z;
        // z = v[n] = v[n] + m;

        y = v[0];

        uint32_t m =
            (z >> 5) ^ (y << 2);

        m +=
            ((y >> 3) ^ (z << 4))
            ^ (d ^ y);

        m +=
            k[(n & 3) ^ e] ^ z;

        z =
            v[n] + m;

        v[n] =
            z;
    }


    return v;
}

static std::string XEncode(
    const std::string &input,
    const std::string &key)
{
    std::vector<uint32_t> v =
        XEncodeToBytes(input, key);

    std::string result;

    result.reserve(
        (v.size() - 1) * 4);

    //JS：
    //
    // a[i] = String.fromCharCode(
    //     a[i] & 0xff,
    //     a[i] >>> 8 & 0xff,
    //     a[i] >>> 16 & 0xff,
    //     a[i] >>> 24 & 0xff
    // );

    for (uint32_t value : v)
    {
        result.push_back(
            static_cast<char>(value & 0xFF));

        result.push_back(
            static_cast<char>((value >> 8) & 0xFF));

        result.push_back(
            static_cast<char>((value >> 16) & 0xFF));

        result.push_back(
            static_cast<char>((value >> 24) & 0xFF));
    }

    return result;
}

static std::string Base64Encode(
    const std::string &data)
{
    //Base64 字符表
    //
    // 标准：
    // ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/
    //
    // SRun：
    // LVoJPiCN2R8G90yg+hmFHuacZ1OWMnrsSTXkYpUq/3dlbfKwv6xztjI7DeBE45QA

    static constexpr char standardTable[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    static constexpr char srunTable[] =
        "LVoJPiCN2R8G90yg+hmFHuacZ1OWMnrs"
        "STXkYpUq/3dlbfKwv6xztjI7DeBE45QA";

    std::string result;

    size_t i = 0;

    while (i < data.size())
    {
        uint32_t a =
            static_cast<unsigned char>(data[i++]);

        uint32_t b = 0;
        uint32_t c = 0;

        bool hasB = i < data.size();

        if (hasB)
        {
            b =
                static_cast<unsigned char>(
                    data[i++]);
        }

        bool hasC = i < data.size();

        if (hasC)
        {
            c =
                static_cast<unsigned char>(
                    data[i++]);
        }

        uint32_t triple =
            (a << 16) |
            (b << 8) |
            c;

        char c1 =
            standardTable[(triple >> 18) & 0x3F];

        char c2 =
            standardTable[(triple >> 12) & 0x3F];

        char c3 =
            standardTable[(triple >> 6) & 0x3F];

        char c4 =
            standardTable[triple & 0x3F];

        // 将普通 Base64 字符映射到 SRun 字符表
        auto convert = [&](char c)
        {
            for (int j = 0; j < 64; ++j)
            {
                if (standardTable[j] == c)
                    return srunTable[j];
            }

            return c;
        };

        result += convert(c1);
        result += convert(c2);

        if (hasB)
            result += convert(c3);
        else
            result += '=';

        if (hasC)
            result += convert(c4);
        else
            result += '=';
    }

    return result;
}

bool LoginCampusNetwork()
{
    const std::wstring host =
        L"219.242.208.131";

    //校园网信息

    const std::string username =
        "";

    const std::string password =
        "";

    std::string ip = GetWifiIPv4();

    const std::string acId =
        "2";

    const std::string doubleStack =
        "0";

    // SRun JS:
    //
    // var enc = "srun_bx1";
    // var n = 200;
    // var type = 1;

    const std::string enc =
        "srun_bx1";

    const std::string n =
        "200";

    const std::string type =
        "1";

    //get_challenge

    std::wstring challengePath =
        L"/cgi-bin/get_challenge"
        L"?callback=test"
        L"&username=" +
        std::wstring(
            username.begin(),
            username.end()) +
        L"&ip=" +
        std::wstring(
            ip.begin(),
            ip.end());

    DWORD challengeStatus{};

    std::string challengeResponse =
        HttpGet(
            host,
            challengePath,
            challengeStatus);

    std::cout
        << "Challenge HTTP Status: "
        << challengeStatus
        << '\n';

    std::cout
        << "Challenge Response: "
        << challengeResponse
        << '\n';

    if (challengeStatus != 200)
    {
        std::cout
            << "get_challenge failed\n";

        return false;
    }

    //提取 challenge
    //
    // test({
    //     "challenge":"xxxxxxxx",
    //     ...
    // })

    const std::string challengeKey =
        "\"challenge\":\"";

    size_t challengeBegin =
        challengeResponse.find(
            challengeKey);

    if (challengeBegin ==
        std::string::npos)
    {
        std::cout
            << "Challenge not found\n";

        return false;
    }

    challengeBegin +=
        challengeKey.size();

    size_t challengeEnd =
        challengeResponse.find(
            '"',
            challengeBegin);

    if (challengeEnd ==
        std::string::npos)
    {
        std::cout
            << "Invalid challenge response\n";

        return false;
    }

    const std::string challenge =
        challengeResponse.substr(
            challengeBegin,
            challengeEnd - challengeBegin);

    std::cout
        << "Challenge: "
        << challenge
        << '\n';


    //计算 HMAC-MD5
    //
    // JS:
    //
    // hmd5 = md5(data.password, token)

    // password={MD5} + hmd5

    const std::string hmd5 =
        HmacMd5(
            password,
            challenge);

    if (hmd5.empty())
    {
        std::cout
            << "HMAC-MD5 failed\n";

        return false;
    }

    std::cout
        << "HMAC-MD5: "
        << hmd5
        << '\n';


    // 构造 info
    //
    // JS:
    //
    // info({
    //     username: username,
    //     password: data.password,
    //     ip: data.ip || response.client_ip,
    //     acid: data.ac_id,
    //     enc_ver: enc_ver
    // }, token)
    //
    // ->
    //
    // JSON.stringify(...)
    // ->
    // xEncode(json, token)
    // ->
    // Base64
    // ->
    // {SRBX1}

    std::string json =
        "{"
        "\"username\":\"" +
        username + "\","
                   "\"password\":\"" +
        password + "\","
                   "\"ip\":\"" +
        ip + "\","
             "\"acid\":\"" +
        acId + "\","
               "\"enc_ver\":\"" +
        enc + "\""
              "}";

    std::cout
        << "JSON: "
        << json
        << '\n';

    std::string encoded =
        XEncode(
            json,
            challenge);

    std::string base64 =
        Base64Encode(
            encoded);

    std::string info =
        "{SRBX1}" + base64;

    std::cout
        << "Info: "
        << info
        << '\n';


    //构造 chksum
    //
    // JS 原代码：
    //
    // var chkstr = token + username;
    // chkstr += token + hmd5;
    // chkstr += token + data.ac_id;
    // chkstr += token + ip;
    // chkstr += token + n;
    // chkstr += token + type;
    // chkstr += token + i;
    //
    // chksum = sha1(chkstr)

    std::string chkstr =
        challenge + username +
        challenge + hmd5 +
        challenge + acId +
        challenge + ip +
        challenge + n +
        challenge + type +
        challenge + info;

    const std::string chksum =
        Sha1(chkstr);

    if (chksum.empty())
    {
        std::cout
            << "SHA1 failed\n";

        return false;
    }

    std::cout
        << "Chksum: "
        << chksum
        << '\n';

    //登录请求

    std::string loginQuery =
        "/cgi-bin/srun_portal"
        "?callback=test"
        "&action=login"
        "&username=" +
        UrlEncode(username)

        + "&password=" +
        UrlEncode(
            "{MD5}" + hmd5)

        + "&ac_id=" +
        UrlEncode(acId)

        + "&ip=" +
        UrlEncode(ip)

        + "&chksum=" +
        UrlEncode(chksum)

        + "&info=" +
        UrlEncode(info)

        + "&n=" +
        UrlEncode(n)

        + "&type=" +
        UrlEncode(type)

        + "&os=" +
        UrlEncode("Windows 10")

        + "&name=" +
        UrlEncode("Windows")

        + "&double_stack=" +
        UrlEncode(doubleStack);

    std::wstring loginPath(
        loginQuery.begin(),
        loginQuery.end());

    std::cout
        << "Sending login request...\n";

    //发送登录请求

    DWORD loginStatus{};

    std::string loginResponse =
        HttpGet(
            host,
            loginPath,
            loginStatus);

    std::cout
        << "Login HTTP Status: "
        << loginStatus
        << '\n';

    std::cout
        << "Login Response: "
        << loginResponse
        << '\n';

    if (loginStatus != 200)
    {
        return false;
    }

    //判断返回结果

    if (
        loginResponse.find(
            "\"suc_msg\":\"login_ok\"") != std::string::npos)
    {
        std::cout
            << "Connection successful"
            << '\n';

        return true;
    }

    if (
        loginResponse.find(
            "ip_already_online_error") != std::string::npos)
    {
        std::cout
            << "Already connected, no authentication required"
            << '\n';

        return true;
    }

    std::cout
        << "Connection failed"
        << '\n';

    return false;
}
