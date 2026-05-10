#pragma once

#include <optional>
#include <string>

namespace dev_dash::core
{
    enum class DiffKind
    {
        kUnchanged,
        kModified,
        kMissing,
        kCustom
    };

    struct DiffEntry
    {
        std::string relativePath;
        DiffKind kind = DiffKind::kUnchanged;
        std::optional<std::string> sourceContent;
        std::optional<std::string> targetContent;
    };
}
