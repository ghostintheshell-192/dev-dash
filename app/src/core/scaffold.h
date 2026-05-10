#pragma once

#include <filesystem>
#include <string>

namespace dev_dash::core
{
    struct Scaffold
    {
        std::string name;
        std::filesystem::path path;
        std::string description;
        bool isDefault = false;
    };
}
