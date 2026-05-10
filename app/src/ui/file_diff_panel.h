#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../core/line_diff.h"

namespace dev_dash::ui
{
    class FileDiffPanel
    {
    public:
        // Load the two files, compute the diff, and open the panel.
        // labelA / labelB are shown in the header (e.g. scaffold name, "Project").
        void Open(const std::filesystem::path& pathA, const std::string& labelA,
                  const std::filesystem::path& pathB, const std::string& labelB);

        void Render();

        bool IsOpen() const { return _open; }

    private:
        std::filesystem::path       _pathA;
        std::filesystem::path       _pathB;
        std::string                 _labelA;
        std::string                 _labelB;
        std::string                 _windowTitle;
        std::vector<core::DiffLine> _diff;
        int                         _addedCount   = 0;
        int                         _removedCount = 0;
        bool                        _open         = false;
        bool                        _tooLarge     = false;
    };
}
