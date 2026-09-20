cmake_minimum_required(VERSION 3.20)

project(BSUCampusNetwork)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    BSUCampusNetworkClient
    src/BSUCampusNetworkClient.cpp
    src/Network/Wifi.cpp
    src/Network/Http.cpp
    src/Crypto/Md5.cpp
    src/Crypto/Sha1.cpp
    src/SRun/SRun.cpp
    src/Utils/Url.cpp
    src/Utils/Config.cpp
)

add_executable(
    BSUCampusNetworkInstaller
    src/BSUCampusNetworkInstaller.cpp
)

add_executable(
    BSUCampusNetworkUninstaller
    src/BSUCampusNetworkUninstaller.cpp
)

target_link_libraries(
    BSUCampusNetworkClient
    wlanapi
    wininet
    winhttp
    bcrypt
    iphlpapi
    ws2_32
)

target_compile_options(BSUCampusNetworkClient PRIVATE /utf-8)
target_compile_options(BSUCampusNetworkInstaller PRIVATE /utf-8)
target_compile_options(BSUCampusNetworkUninstaller PRIVATE /utf-8)
