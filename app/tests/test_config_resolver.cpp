#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#include "core/project.h"
#include "services/config_resolver.h"

#include "test_support.h"

// One fixture per Claude Code loading rule: each test builds a fake home and
// a fake project on a tmpdir and checks what the resolver reports. The rules
// follow the Claude Code docs (memory, skills, sub-agents, mcp) as described
// in .development/reference/technical/resource-model.md.

using namespace dev_dash;
using core::ConfigLayerKind;
using core::ConfigSectionKind;
using dev_dash::tests::TempDir;
using dev_dash::tests::WriteFile;

namespace
{
    struct Fixture
    {
        TempDir root{"resolver"};
        std::filesystem::path home    = root / "home";
        std::filesystem::path project = root / "work" / "proj";

        Fixture()
        {
            std::filesystem::create_directories(home);
            std::filesystem::create_directories(project);
        }

        core::EffectiveConfig Resolve() const
        {
            services::ConfigResolver resolver;
            return resolver.Resolve(core::Project(project), home);
        }
    };

    const core::ConfigSection& Section(const core::EffectiveConfig& config,
                                       ConfigSectionKind kind)
    {
        const auto& sections = config.Sections();
        const auto it = std::find_if(sections.begin(), sections.end(),
            [&](const core::ConfigSection& s) { return s.kind == kind; });
        REQUIRE(it != sections.end());
        return *it;
    }

    std::vector<std::string> Labels(const core::ConfigSection& section)
    {
        std::vector<std::string> labels;
        for (const auto& node : section.nodes)
            labels.push_back(node.relativePath);
        return labels;
    }

    const core::ConfigNode& Node(const core::ConfigSection& section,
                                 const std::string& label,
                                 ConfigLayerKind layer)
    {
        const auto it = std::find_if(section.nodes.begin(), section.nodes.end(),
            [&](const core::ConfigNode& n)
            { return n.relativePath == label && n.sourceLayer == layer; });
        REQUIRE(it != section.nodes.end());
        return *it;
    }
}

TEST_CASE("CLAUDE.md at the project root is loaded", "[resolver][claude-md]")
{
    Fixture f;
    WriteFile(f.project / "CLAUDE.md", "root");

    const auto config = f.Resolve();
    const auto& section = Section(config, ConfigSectionKind::kClaudeMd);

    REQUIRE(section.nodes.size() == 1);
    CHECK(section.nodes[0].relativePath == "CLAUDE.md");
    CHECK(section.nodes[0].sourceLayer == ConfigLayerKind::kProject);
}

TEST_CASE("CLAUDE.md chain follows Claude Code's load order", "[resolver][claude-md]")
{
    Fixture f;
    WriteFile(f.home / ".claude/CLAUDE.md", "user");
    WriteFile(f.root / "work/CLAUDE.md", "ancestor");
    WriteFile(f.project / "CLAUDE.md", "project root");
    WriteFile(f.project / ".claude/CLAUDE.md", "project .claude");
    WriteFile(f.project / "CLAUDE.local.md", "personal");

    const auto config = f.Resolve();
    const auto& section = Section(config, ConfigSectionKind::kClaudeMd);

    CHECK(Labels(section) == std::vector<std::string>{
        "CLAUDE.md", "../CLAUDE.md", "CLAUDE.md", ".claude/CLAUDE.md", "CLAUDE.local.md"});
    CHECK(section.nodes[0].sourceLayer == ConfigLayerKind::kGlobal);
    CHECK(section.nodes[1].sourceLayer == ConfigLayerKind::kAncestor);
    CHECK(section.nodes[2].sourceLayer == ConfigLayerKind::kProject);
    CHECK(section.nodes[3].sourceLayer == ConfigLayerKind::kProject);
    CHECK(section.nodes[4].sourceLayer == ConfigLayerKind::kLocal);
}

TEST_CASE("@-includes follow the file that includes them", "[resolver][claude-md]")
{
    Fixture f;
    WriteFile(f.project / ".claude/CLAUDE.md", "intro\n@../docs/arch.md\n");
    WriteFile(f.project / "docs/arch.md", "arch");

    const auto config = f.Resolve();
    const auto& section = Section(config, ConfigSectionKind::kClaudeMd);

    CHECK(Labels(section) == std::vector<std::string>{".claude/CLAUDE.md", "docs/arch.md"});
    CHECK(section.nodes[1].sourceLayer == ConfigLayerKind::kProject);
}

TEST_CASE("Skills are directories holding SKILL.md", "[resolver][skills]")
{
    Fixture f;
    WriteFile(f.project / ".claude/skills/session-handoff/SKILL.md", "skill");
    WriteFile(f.project / ".claude/skills/loose-file.md", "not a skill");
    WriteFile(f.project / ".claude/skills/no-skill-md/notes.md", "not a skill");

    const auto config = f.Resolve();
    const auto& section = Section(config, ConfigSectionKind::kSkills);

    CHECK(Labels(section) == std::vector<std::string>{"session-handoff"});
    CHECK(section.nodes[0].sourceFilePath
          == f.project / ".claude/skills/session-handoff/SKILL.md");
}

TEST_CASE("A user skill shadows a project skill with the same name", "[resolver][skills]")
{
    Fixture f;
    WriteFile(f.home / ".claude/skills/deploy/SKILL.md", "user");
    WriteFile(f.project / ".claude/skills/deploy/SKILL.md", "project");

    const auto config = f.Resolve();
    const auto& section = Section(config, ConfigSectionKind::kSkills);

    REQUIRE(section.nodes.size() == 2);
    CHECK_FALSE(Node(section, "deploy", ConfigLayerKind::kGlobal).shadowed);
    const auto loser = Node(section, "deploy", ConfigLayerKind::kProject);
    CHECK(loser.shadowed);
    CHECK(loser.note == "shadowed by Global");
}

TEST_CASE("A project agent shadows a user agent with the same name", "[resolver][agents]")
{
    Fixture f;
    // Different file names, same frontmatter name: identity is the name.
    WriteFile(f.home / ".claude/agents/reviewer.md", "---\nname: reviewer\n---\nuser");
    WriteFile(f.project / ".claude/agents/code-reviewer.md",
              "---\nname: \"reviewer\"\ndescription: x\n---\nproject");

    const auto config = f.Resolve();
    const auto& section = Section(config, ConfigSectionKind::kAgents);

    REQUIRE(section.nodes.size() == 2);
    CHECK(Node(section, "reviewer.md", ConfigLayerKind::kGlobal).shadowed);
    CHECK(Node(section, "reviewer.md", ConfigLayerKind::kGlobal).note == "shadowed by Project");
    CHECK_FALSE(Node(section, "code-reviewer.md", ConfigLayerKind::kProject).shadowed);
}

TEST_CASE("MCP servers come from ~/.claude.json and .mcp.json", "[resolver][mcp]")
{
    Fixture f;
    const std::string projectKey = f.project.string();
    WriteFile(f.home / ".claude.json",
              R"({"mcpServers": {"user-only": {}, "shared": {}},
                  "projects": {")" + projectKey + R"(": {"mcpServers": {"shared": {}, "local-only": {}}},
                               "/some/other/project": {"mcpServers": {"elsewhere": {}}}}})");
    WriteFile(f.project / ".mcp.json", R"({"mcpServers": {"shared": {}, "team": {}}})");
    // Not a Claude Code source for MCP servers: must be ignored.
    WriteFile(f.project / ".claude/settings.json", R"({"mcpServers": {"bogus": {}}})");

    const auto config = f.Resolve();
    const auto& section = Section(config, ConfigSectionKind::kMcpServers);

    auto labels = Labels(section);
    std::sort(labels.begin(), labels.end());
    CHECK(labels == std::vector<std::string>{
        "local-only", "shared", "shared", "shared", "team", "user-only"});

    SECTION("local beats project, project beats user")
    {
        CHECK_FALSE(Node(section, "shared", ConfigLayerKind::kLocal).shadowed);
        CHECK(Node(section, "shared", ConfigLayerKind::kProject).note == "shadowed by Local");
        CHECK(Node(section, "shared", ConfigLayerKind::kGlobal).note == "shadowed by Local");
    }

    SECTION("servers defined once are not shadowed")
    {
        CHECK_FALSE(Node(section, "user-only", ConfigLayerKind::kGlobal).shadowed);
        CHECK_FALSE(Node(section, "team", ConfigLayerKind::kProject).shadowed);
        CHECK_FALSE(Node(section, "local-only", ConfigLayerKind::kLocal).shadowed);
    }
}

TEST_CASE("A project MCP server shadows a user one", "[resolver][mcp]")
{
    Fixture f;
    WriteFile(f.home / ".claude.json", R"({"mcpServers": {"db": {}}})");
    WriteFile(f.project / ".mcp.json", R"({"mcpServers": {"db": {}}})");

    const auto config = f.Resolve();
    const auto& section = Section(config, ConfigSectionKind::kMcpServers);

    CHECK_FALSE(Node(section, "db", ConfigLayerKind::kProject).shadowed);
    CHECK(Node(section, "db", ConfigLayerKind::kGlobal).note == "shadowed by Project");
}

TEST_CASE("Memory path encodes every non-alphanumeric character", "[resolver][memory]")
{
    Fixture f;
    const std::filesystem::path project = f.root / "my_repos" / "dev.dash";
    std::filesystem::create_directories(project);

    std::string encoded = project.string();
    for (char& c : encoded)
        if (!std::isalnum(static_cast<unsigned char>(c))) c = '-';
    WriteFile(f.home / ".claude/projects" / encoded / "memory/MEMORY.md", "facts");

    services::ConfigResolver resolver;
    const auto config = resolver.Resolve(core::Project(project), f.home);
    const auto& section = Section(config, ConfigSectionKind::kMemory);

    REQUIRE(section.nodes.size() == 1);
    CHECK(section.nodes[0].relativePath == "MEMORY.md");
}

TEST_CASE("Hooks from every settings file add up", "[resolver][hooks]")
{
    Fixture f;
    WriteFile(f.home / ".claude/settings.json",
              R"({"hooks": {"SessionStart": [{"hooks": []}]}})");
    WriteFile(f.project / ".claude/settings.json",
              R"({"hooks": {"PreToolUse": [{"matcher": "Bash", "hooks": []}]}})");
    WriteFile(f.project / ".claude/settings.local.json",
              R"({"hooks": {"SessionStart": [{"hooks": []}]}})");

    const auto config = f.Resolve();
    const auto& section = Section(config, ConfigSectionKind::kHooks);

    CHECK(Labels(section) == std::vector<std::string>{
        "SessionStart", "PreToolUse / Bash", "SessionStart"});
    for (const auto& node : section.nodes)
        CHECK_FALSE(node.shadowed);
}

TEST_CASE("Without a home directory only project sources are read", "[resolver]")
{
    Fixture f;
    WriteFile(f.home / ".claude/CLAUDE.md", "user");
    WriteFile(f.project / "CLAUDE.md", "project");

    services::ConfigResolver resolver;
    const auto config = resolver.Resolve(core::Project(f.project), {});

    CHECK(Labels(Section(config, ConfigSectionKind::kClaudeMd))
          == std::vector<std::string>{"CLAUDE.md"});
    CHECK(Section(config, ConfigSectionKind::kMemory).nodes.empty());
}
