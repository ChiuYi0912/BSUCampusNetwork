#pragma once

#include <string>

std::string HmacMd5(
    const std::string &data,
    const std::string &key);
