#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

namespace dev_dash::tests
{
    // RAII scratch directory under the system temp dir. Created on construction,
    // recursively removed on destruction. Each instance gets a unique name so
    // tests never collide, even when run in parallel by ctest.
    class TempDir
    {
    public:
        explicit TempDir(std::string_view label = "devdash-test")
        {
            static int counter = 0;
            const auto base = std::filesystem::temp_directory_path();
            _path = base / (std::string(label) + "-" + std::to_string(++counter)
                            + "-" + std::to_string(reinterpret_cast<std::uintptr_t>(this)));
            std::filesystem::create_directories(_path);
        }

        ~TempDir()
        {
            std::error_code ec;
            std::filesystem::remove_all(_path, ec);
        }

        TempDir(const TempDir&)            = delete;
        TempDir& operator=(const TempDir&) = delete;

        const std::filesystem::path& Path() const { return _path; }
        std::filesystem::path operator/(std::string_view rel) const { return _path / rel; }

    private:
        std::filesystem::path _path;
    };

    // Write `content` to `path`, creating parent directories as needed.
    inline void WriteFile(const std::filesystem::path& path, std::string_view content)
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream f(path);
        f << content;
    }

    // Read the whole file back as a string ("" if absent).
    inline std::string ReadFile(const std::filesystem::path& path)
    {
        std::ifstream f(path);
        return std::string(std::istreambuf_iterator<char>(f),
                           std::istreambuf_iterator<char>());
    }
}
