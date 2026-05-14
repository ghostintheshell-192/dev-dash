#pragma once

#include <cctype>
#include <filesystem>
#include <string>

namespace dev_dash::core
{
    struct Project
    {
        std::filesystem::path path;
        bool hasClaudeDir = false;
        bool hasGit       = false;
        bool hasClaudeMd  = false;

        Project() = default;
        explicit Project(const std::filesystem::path& projectPath);

        void RefreshPresenceFlags();
    };

    inline Project::Project(const std::filesystem::path& projectPath)
        : path(projectPath)
    {
        RefreshPresenceFlags();
    }

    inline void Project::RefreshPresenceFlags()
    {
        hasClaudeDir = std::filesystem::exists(path / ".claude");
        hasGit       = std::filesystem::exists(path / ".git");
        hasClaudeMd  = std::filesystem::exists(path / "CLAUDE.md");
    }

    // Derive a filesystem-safe slug from the project's last path component.
    inline std::string MakeProjectSlug(const std::filesystem::path& projectPath)
    {
        std::string s = projectPath.filename().string();
        for (char& c : s)
            if (!std::isalnum(static_cast<unsigned char>(c))
                && c != '-' && c != '_')
                c = '-';
        return s;
    }
}
