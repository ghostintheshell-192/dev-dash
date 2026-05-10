#pragma once

#include <filesystem>

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
}
