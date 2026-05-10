#include "promote_engine.h"

#include <system_error>

namespace dev_dash::services
{
    PromoteEngine::Result PromoteEngine::Promote(
        const std::filesystem::path&    projectRoot,
        const std::filesystem::path&    scaffoldRoot,
        const std::vector<std::string>& relativePaths) const
    {
        Result result;

        for (const auto& rel : relativePaths)
        {
            const auto src = projectRoot  / rel;
            const auto dst = scaffoldRoot / rel;

            std::error_code ec;

            std::filesystem::create_directories(dst.parent_path(), ec);
            if (ec)
            {
                result.errors.push_back("mkdir " + dst.parent_path().string()
                                        + ": " + ec.message());
                continue;
            }

            std::filesystem::copy_file(src, dst,
                std::filesystem::copy_options::overwrite_existing, ec);
            if (ec)
            {
                result.errors.push_back("copy " + rel + ": " + ec.message());
                continue;
            }

            ++result.copiedCount;
        }

        return result;
    }
}
