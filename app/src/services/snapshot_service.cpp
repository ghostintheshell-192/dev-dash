#include "snapshot_service.h"
#include "apply_engine.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <fstream>
#include <optional>
#include <string>

namespace dev_dash::services
{
    namespace
    {
        // Current local time as "YYYYMMDD-HHMMSS" — sortable lexicographically.
        std::string NowTimestamp()
        {
            std::time_t t = std::time(nullptr);
            char buf[16];
            std::strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S", std::localtime(&t));
            return buf;
        }

        // Sanitize an arbitrary name to a filesystem-safe slug.
        std::string NameToSlug(std::string_view name)
        {
            std::string s(name);
            for (char& c : s)
                if (!std::isalnum(static_cast<unsigned char>(c))
                    && c != '-' && c != '_')
                    c = '-';
            return s;
        }

        // Recursively copy config-relevant paths from projectRoot into snapshotDir.
        // Partial captures are acceptable; the snapshot dir is created if needed.
        bool CopyConfigFiles(const std::filesystem::path& projectRoot,
                             const std::filesystem::path& snapshotDir)
        {
            std::error_code ec;
            std::filesystem::create_directories(snapshotDir, ec);
            if (ec) return false;

            static const char* kPaths[] = {"CLAUDE.md", ".claude", ".githooks"};
            for (const char* rel : kPaths)
            {
                const auto src = projectRoot / rel;
                if (!std::filesystem::exists(src, ec)) { ec.clear(); continue; }
                ec.clear();

                const auto dst = snapshotDir / rel;
                if (std::filesystem::is_directory(src, ec))
                {
                    ec.clear();
                    std::filesystem::copy(src, dst,
                        std::filesystem::copy_options::recursive
                        | std::filesystem::copy_options::overwrite_existing, ec);
                }
                else
                {
                    ec.clear();
                    std::filesystem::copy_file(src, dst,
                        std::filesystem::copy_options::overwrite_existing, ec);
                }
                ec.clear();
            }
            return true;
        }

        void WriteMeta(const std::filesystem::path& snapshotDir,
                       core::SnapshotKind kind,
                       std::string_view name,
                       std::string_view description,
                       const std::optional<std::string>& action)
        {
            std::ofstream f(snapshotDir / ".devdash-snapshot");
            f << "kind=" << (kind == core::SnapshotKind::kAuto ? "auto" : "explicit") << '\n';
            f << "name=" << name << '\n';
            f << "description=" << description << '\n';
            if (action) f << "action=" << *action << '\n';
        }

        core::Snapshot ParseSnapshotDir(const std::filesystem::path& dir,
                                        std::string_view projectSlug)
        {
            core::Snapshot s;
            s.projectSlug = std::string(projectSlug);
            s.path        = dir;
            s.name        = dir.filename().string();

            // Timestamp = first 15 chars of dir name ("YYYYMMDD-HHMMSS")
            const std::string dirName = dir.filename().string();
            s.timestamp = dirName.substr(0, std::min<size_t>(dirName.size(), 15));

            const auto metaPath = dir / ".devdash-snapshot";
            if (!std::filesystem::exists(metaPath)) return s;

            std::ifstream f(metaPath);
            std::string line;
            while (std::getline(f, line))
            {
                if      (line.compare(0, 5,  "kind=")        == 0)
                    s.kind = (line.substr(5) == "auto")
                        ? core::SnapshotKind::kAuto : core::SnapshotKind::kExplicit;
                else if (line.compare(0, 5,  "name=")        == 0)
                    s.name = line.substr(5);
                else if (line.compare(0, 12, "description=") == 0)
                    s.description = line.substr(12);
                else if (line.compare(0, 7,  "action=")      == 0)
                    s.originatingAction = line.substr(7);
            }
            return s;
        }
    }

    SnapshotService::SnapshotService(ApplyEngine& applyEngine)
        : _applyEngine(applyEngine)
    {
    }

    core::Snapshot SnapshotService::SaveExplicit(
        const core::Project& project,
        std::string_view name,
        std::string_view description)
    {
        if (_snapshotRoot.empty()) return {};
        const std::string slug = core::MakeProjectSlug(project.path);
        const auto dir = _snapshotRoot / slug
                       / (NowTimestamp() + "-" + NameToSlug(name));

        if (!CopyConfigFiles(project.path, dir)) return {};
        WriteMeta(dir, core::SnapshotKind::kExplicit, name, description, std::nullopt);
        return ParseSnapshotDir(dir, slug);
    }

    core::Snapshot SnapshotService::SaveAuto(
        const core::Project& project,
        std::string_view action)
    {
        if (_snapshotRoot.empty()) return {};
        const std::string slug = core::MakeProjectSlug(project.path);
        const auto dir = _snapshotRoot / slug
                       / (NowTimestamp() + "-auto-" + NameToSlug(action));

        if (!CopyConfigFiles(project.path, dir)) return {};

        const std::string autoName = "auto-" + std::string(action);
        WriteMeta(dir, core::SnapshotKind::kAuto, autoName, "", std::string(action));

        PruneAuto(slug);
        return ParseSnapshotDir(dir, slug);
    }

    std::vector<core::Snapshot> SnapshotService::List(std::string_view projectSlug)
    {
        std::vector<core::Snapshot> result;
        const auto dir = _snapshotRoot / std::string(projectSlug);
        if (!std::filesystem::exists(dir)) return result;

        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
        {
            if (!entry.is_directory(ec) || ec) { ec.clear(); continue; }
            result.push_back(ParseSnapshotDir(entry.path(), projectSlug));
        }

        // Newest first (dir names are timestamp-prefixed → reverse lex order)
        std::sort(result.begin(), result.end(),
            [](const core::Snapshot& a, const core::Snapshot& b)
            { return a.path.filename().string() > b.path.filename().string(); });

        return result;
    }

    bool SnapshotService::Restore(
        const core::Snapshot& snapshot,
        const core::Project& target)
    {
        if(!std::filesystem::exists(snapshot.path))
            return false;
        // Save current state before overwriting
        SaveAuto(target, "pre-restore");

        ApplyEngine::ApplyConfig cfg;
        cfg.createAutosnapshot = false;
        cfg.forceOverwrite     = true;

        std::error_code ec;
        for (const auto& entry :
             std::filesystem::recursive_directory_iterator(snapshot.path, ec))
        {
            if (!entry.is_regular_file(ec) || ec) { ec.clear(); continue; }
            const auto rel = std::filesystem::relative(entry.path(), snapshot.path, ec);
            if (ec) { ec.clear(); continue; }
            if (rel.filename() == ".devdash-snapshot") continue;
            cfg.filesToApply.push_back(rel.string());
        }

        if(cfg.filesToApply.empty())
            return false;

        const auto result = _applyEngine.Apply(snapshot.path, target.path, cfg);
        return result.failed == 0 && result.applied > 0;
    }

    int SnapshotService::PruneAuto(std::string_view projectSlug, int maxAutoSnapshots)
    {
        auto all = List(projectSlug);

        // all is newest-first; collect auto-snapshot paths
        std::vector<std::filesystem::path> autoPaths;
        for (const auto& s : all)
            if (s.kind == core::SnapshotKind::kAuto)
                autoPaths.push_back(s.path);

        int pruned = 0;
        for (int i = maxAutoSnapshots; i < static_cast<int>(autoPaths.size()); ++i)
        {
            std::error_code ec;
            std::filesystem::remove_all(autoPaths[i], ec);
            if (!ec) ++pruned;
        }
        return pruned;
    }
}
