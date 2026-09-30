#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace dev_dash::services
{
    class PromoteEngine
    {
    public:
        struct Result
        {
            int                      copiedCount = 0;
            std::vector<std::string> errors;

            bool Ok() const { return errors.empty(); }
        };

        // Copy each relative path from projectRoot to scaffoldRoot,
        // creating intermediate directories as needed.
        Result Promote(
            const std::filesystem::path&      projectRoot,
            const std::filesystem::path&      scaffoldRoot,
            const std::vector<std::string>&   relativePaths) const;
    };
}
