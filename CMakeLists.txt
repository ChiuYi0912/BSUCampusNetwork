cmake_minimum_required(VERSION 3.20)

project(BSUCampusNetwork)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    BSUCampusNetworkClient
    src/BSUCampusNetworkClient.cpp
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
)