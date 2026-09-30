#include <catch2/catch_test_macros.hpp>

#include "services/apply_engine.h"
#include "services/snapshot_service.h"

#include "test_support.h"

using namespace dev_dash;
using dev_dash::services::ApplyEngine;
using dev_dash::tests::TempDir;
using dev_dash::tests::WriteFile;
using dev_dash::tests::ReadFile;

TEST_CASE("ApplyEngine copies new files into an empty target", "[apply]")
{
    TempDir source("apply-src");
    TempDir target("apply-dst");

    WriteFile(source / "CLAUDE.md", "hello");
    WriteFile(source / ".claude/rules/overview.md", "rules");

    ApplyEngine engine;
    ApplyEngine::ApplyConfig cfg;
    cfg.createAutosnapshot = false;
    cfg.filesToApply = {"CLAUDE.md", ".claude/rules/overview.md"};

    const auto result = engine.Apply(source.Path(), target.Path(), cfg);

    CHECK(result.applied == 2);
    CHECK(result.skipped == 0);
    CHECK(result.failed == 0);
    CHECK(ReadFile(target / "CLAUDE.md") == "hello");
    CHECK(ReadFile(target / ".claude/rules/overview.md") == "rules");
}

TEST_CASE("ApplyEngine skips an existing file unless forceOverwrite is set", "[apply]")
{
    TempDir source("apply-src");
    TempDir target("apply-dst");

    WriteFile(source / "CLAUDE.md", "new content");
    WriteFile(target / "CLAUDE.md", "old content");

    ApplyEngine engine;
    ApplyEngine::ApplyConfig cfg;
    cfg.createAutosnapshot = false;
    cfg.filesToApply = {"CLAUDE.md"};

    SECTION("without forceOverwrite the destination is preserved")
    {
        const auto result = engine.Apply(source.Path(), target.Path(), cfg);
        CHECK(result.skipped == 1);
        CHECK(result.applied == 0);
        CHECK(ReadFile(target / "CLAUDE.md") == "old content");
    }

    SECTION("with forceOverwrite the destination is replaced")
    {
        cfg.forceOverwrite = true;
        const auto result = engine.Apply(source.Path(), target.Path(), cfg);
        CHECK(result.applied == 1);
        CHECK(result.skipped == 0);
        CHECK(ReadFile(target / "CLAUDE.md") == "new content");
    }
}

TEST_CASE("ApplyEngine skips files missing from the source", "[apply]")
{
    TempDir source("apply-src");
    TempDir target("apply-dst");

    WriteFile(source / "CLAUDE.md", "present");

    ApplyEngine engine;
    ApplyEngine::ApplyConfig cfg;
    cfg.createAutosnapshot = false;
    cfg.filesToApply = {"CLAUDE.md", "does-not-exist.md"};

    const auto result = engine.Apply(source.Path(), target.Path(), cfg);

    CHECK(result.applied == 1);
    CHECK(result.skipped == 1);
    CHECK(result.failed == 0);
}

TEST_CASE("ApplyEngine creates nested destination directories", "[apply]")
{
    TempDir source("apply-src");
    TempDir target("apply-dst");

    WriteFile(source / ".claude/hooks/pre-commit.d/00-check.sh", "#!/bin/sh");

    ApplyEngine engine;
    ApplyEngine::ApplyConfig cfg;
    cfg.createAutosnapshot = false;
    cfg.filesToApply = {".claude/hooks/pre-commit.d/00-check.sh"};

    const auto result = engine.Apply(source.Path(), target.Path(), cfg);

    CHECK(result.applied == 1);
    CHECK(std::filesystem::exists(target / ".claude/hooks/pre-commit.d/00-check.sh"));
}

TEST_CASE("ApplyEngine triggers an auto-snapshot when configured", "[apply]")
{
    TempDir source("apply-src");
    TempDir target("myproject");   // dir name becomes the project slug
    TempDir snapshots("apply-snaps");

    // The target must hold something snapshot-worthy (CLAUDE.md / .claude /
    // .githooks) for the auto-snapshot to capture a file.
    WriteFile(target / "CLAUDE.md", "target state");
    WriteFile(source / "CLAUDE.md", "incoming");

    ApplyEngine engine;
    services::SnapshotService snapshots_service(engine);
    snapshots_service.SetSnapshotRoot(snapshots.Path());

    ApplyEngine::ApplyConfig cfg;
    cfg.createAutosnapshot = true;
    cfg.autosnapshotAction = "pre-apply";
    cfg.forceOverwrite     = true;
    cfg.filesToApply       = {"CLAUDE.md"};

    const auto result = engine.Apply(source.Path(), target.Path(), cfg, &snapshots_service);

    CHECK(result.applied == 1);

    const auto slug = core::MakeProjectSlug(target.Path());
    const auto saved = snapshots_service.List(slug);
    REQUIRE(saved.size() == 1);
    CHECK(saved.front().kind == core::SnapshotKind::kAuto);
    CHECK(saved.front().originatingAction == "pre-apply");
}
