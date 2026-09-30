#include <catch2/catch_test_macros.hpp>

#include "services/snapshot_service.h"
#include "services/apply_engine.h"
#include "core/project.h"

#include "test_support.h"

#include <string>

using namespace dev_dash;
using dev_dash::services::ApplyEngine;
using dev_dash::services::SnapshotService;
using dev_dash::tests::TempDir;
using dev_dash::tests::WriteFile;
using dev_dash::tests::ReadFile;

namespace
{
    // Create a snapshot directory by hand under <root>/<slug>/<dirName> with a
    // matching .devdash-snapshot metadata file. Lets PruneAuto/List tests build
    // a deterministic set of snapshots without depending on the wall clock
    // (real saves are timestamped to the second, which would collide).
    void MakeSnapshotDir(const std::filesystem::path& root,
                         const std::string& slug,
                         const std::string& dirName,
                         bool isAuto)
    {
        const auto dir = root / slug / dirName;
        std::filesystem::create_directories(dir);
        WriteFile(dir / ".devdash-snapshot",
                  std::string("kind=") + (isAuto ? "auto" : "explicit") + "\n"
                  + "name=" + dirName + "\n"
                  + (isAuto ? "action=test\n" : "description=desc\n"));
    }

    std::string Pad4(int n)
    {
        std::string s = std::to_string(n);
        return std::string(s.size() < 4 ? 4 - s.size() : 0, '0') + s;
    }
}

TEST_CASE("SaveExplicit captures config files and metadata", "[snapshot]")
{
    TempDir project("snap-proj");
    TempDir snapshots("snap-root");

    WriteFile(project / "CLAUDE.md", "project config");
    WriteFile(project / ".claude/rules/overview.md", "rules");

    ApplyEngine engine;
    SnapshotService service(engine);
    service.SetSnapshotRoot(snapshots.Path());

    const core::Project proj(project.Path());
    const auto snap = service.SaveExplicit(proj, "Before edit", "manual checkpoint");

    CHECK(snap.kind == core::SnapshotKind::kExplicit);
    CHECK(snap.name == "Before edit");
    CHECK(snap.description == "manual checkpoint");
    CHECK_FALSE(snap.path.empty());

    // Config was actually copied; the metadata sidecar exists.
    CHECK(ReadFile(snap.path / "CLAUDE.md") == "project config");
    CHECK(ReadFile(snap.path / ".claude/rules/overview.md") == "rules");
    CHECK(std::filesystem::exists(snap.path / ".devdash-snapshot"));

    const auto slug = core::MakeProjectSlug(project.Path());
    CHECK(service.List(slug).size() == 1);
}

TEST_CASE("SaveAuto records kind and originating action", "[snapshot]")
{
    TempDir project("snap-proj");
    TempDir snapshots("snap-root");

    WriteFile(project / "CLAUDE.md", "config");

    ApplyEngine engine;
    SnapshotService service(engine);
    service.SetSnapshotRoot(snapshots.Path());

    const core::Project proj(project.Path());
    const auto snap = service.SaveAuto(proj, "pre-apply");

    CHECK(snap.kind == core::SnapshotKind::kAuto);
    CHECK(snap.originatingAction == "pre-apply");
    CHECK(snap.name == "auto-pre-apply");
}

TEST_CASE("List returns snapshots newest first", "[snapshot]")
{
    TempDir snapshots("snap-root");
    ApplyEngine engine;
    SnapshotService service(engine);
    service.SetSnapshotRoot(snapshots.Path());

    const std::string slug = "demo";
    MakeSnapshotDir(snapshots.Path(), slug, "20260101-000000-first", false);
    MakeSnapshotDir(snapshots.Path(), slug, "20260102-000000-second", true);
    MakeSnapshotDir(snapshots.Path(), slug, "20260103-000000-third", false);

    const auto all = service.List(slug);
    REQUIRE(all.size() == 3);
    CHECK(all[0].path.filename().string() == "20260103-000000-third");
    CHECK(all[1].path.filename().string() == "20260102-000000-second");
    CHECK(all[2].path.filename().string() == "20260101-000000-first");
}

TEST_CASE("Restore copies snapshot files into the target, minus the metadata", "[snapshot]")
{
    TempDir snapshots("snap-root");
    TempDir target("restore-target");

    ApplyEngine engine;
    SnapshotService service(engine);
    service.SetSnapshotRoot(snapshots.Path());

    // Hand-build a snapshot dir holding config + the metadata sidecar.
    const std::string slug = core::MakeProjectSlug(target.Path());
    const auto snapDir = snapshots.Path() / slug / "20260101-000000-saved";
    WriteFile(snapDir / "CLAUDE.md", "snapshotted config");
    WriteFile(snapDir / ".claude/rules/overview.md", "snapshotted rules");
    WriteFile(snapDir / ".devdash-snapshot", "kind=explicit\nname=saved\n");

    core::Snapshot snap;
    snap.path = snapDir;
    snap.kind = core::SnapshotKind::kExplicit;

    const core::Project targetProject(target.Path());
    const bool ok = service.Restore(snap, targetProject);

    CHECK(ok);
    CHECK(ReadFile(target / "CLAUDE.md") == "snapshotted config");
    CHECK(ReadFile(target / ".claude/rules/overview.md") == "snapshotted rules");
    // The metadata sidecar must NOT leak into the restored project.
    CHECK_FALSE(std::filesystem::exists(target / ".devdash-snapshot"));
}

TEST_CASE("PruneAuto keeps the newest N auto snapshots and leaves explicit ones", "[snapshot]")
{
    TempDir snapshots("snap-root");
    ApplyEngine engine;
    SnapshotService service(engine);
    service.SetSnapshotRoot(snapshots.Path());

    const std::string slug = "demo";

    // 12 auto snapshots (ordinally named so lexicographic sort == age order)
    // plus 2 explicit ones that must survive any prune.
    for (int i = 0; i < 12; ++i)
        MakeSnapshotDir(snapshots.Path(), slug, "auto-" + Pad4(i), true);
    MakeSnapshotDir(snapshots.Path(), slug, "keep-explicit-a", false);
    MakeSnapshotDir(snapshots.Path(), slug, "keep-explicit-b", false);

    const int pruned = service.PruneAuto(slug, 10);

    CHECK(pruned == 2);

    const auto remaining = service.List(slug);
    int autoCount = 0, explicitCount = 0;
    for (const auto& s : remaining)
        (s.kind == core::SnapshotKind::kAuto ? autoCount : explicitCount)++;

    CHECK(autoCount == 10);
    CHECK(explicitCount == 2);

    // The two oldest auto snapshots (auto-0000, auto-0001) are the ones removed.
    CHECK_FALSE(std::filesystem::exists(snapshots.Path() / slug / "auto-0000"));
    CHECK_FALSE(std::filesystem::exists(snapshots.Path() / slug / "auto-0001"));
    CHECK(std::filesystem::exists(snapshots.Path() / slug / "auto-0011"));
}
