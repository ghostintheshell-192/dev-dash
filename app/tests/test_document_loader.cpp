#include <catch2/catch_test_macros.hpp>

#include "services/document_loader.h"

#include "test_support.h"

// One test per kind of link a document can hold: the loader decides where a
// click leads, the panel host only acts on the answer.

using namespace dev_dash;
using services::DocumentLoader;
using services::LinkTarget;
using dev_dash::tests::TempDir;
using dev_dash::tests::WriteFile;

namespace
{
    struct Fixture
    {
        TempDir root{"links"};
        std::filesystem::path docs = std::filesystem::weakly_canonical(root.Path()) / "docs";
        std::filesystem::path from = docs / "guide" / "index.md";
        DocumentLoader loader;

        Fixture()
        {
            WriteFile(from, "# Index\n");
        }

        LinkTarget Resolve(std::string_view url) const { return loader.ResolveLink(from, url); }
    };
}

TEST_CASE("an @-import link opens the imported file", "[links]")
{
    Fixture f;
    const auto target = f.Resolve("claudeimport:///home/u/.claude/CLAUDE.md");
    CHECK(target.kind == LinkTarget::Kind::kImport);
    CHECK(target.path == "/home/u/.claude/CLAUDE.md");
}

TEST_CASE("a relative link to a markdown file resolves against the document", "[links]")
{
    Fixture f;
    WriteFile(f.docs / "reference" / "adr-010.md", "# ADR\n");

    const auto target = f.Resolve("../reference/adr-010.md");
    CHECK(target.kind == LinkTarget::Kind::kDocument);
    CHECK(target.path == f.docs / "reference" / "adr-010.md");
}

TEST_CASE("fragment and query are dropped, percent-escapes decoded", "[links]")
{
    Fixture f;
    WriteFile(f.docs / "guide" / "two words.MD", "# Two\n");

    const auto target = f.Resolve("two%20words.MD#section?x=1");
    CHECK(target.kind == LinkTarget::Kind::kDocument);
    CHECK(target.path == f.docs / "guide" / "two words.MD");
}

TEST_CASE("an existing file that is not markdown opens outside the app", "[links]")
{
    Fixture f;
    WriteFile(f.docs / "guide" / "main.cpp", "int main() {}\n");

    CHECK(f.Resolve("main.cpp").kind == LinkTarget::Kind::kFile);
    CHECK(f.Resolve("..").kind == LinkTarget::Kind::kFile);
}

TEST_CASE("a file:// link is treated as an absolute path", "[links]")
{
    Fixture f;
    const auto target = f.Resolve("file://" + f.from.string());
    CHECK(target.kind == LinkTarget::Kind::kDocument);
    CHECK(target.path == f.from);
}

TEST_CASE("a URL with a scheme opens outside the app", "[links]")
{
    Fixture f;
    for (const std::string_view url : {"https://example.org/a.md", "mailto:someone@example.org"})
    {
        const auto target = f.Resolve(url);
        CHECK(target.kind == LinkTarget::Kind::kExternal);
        CHECK(target.url == url);
    }
}

TEST_CASE("an anchor stays within the document", "[links]")
{
    Fixture f;
    CHECK(f.Resolve("#next-steps").kind == LinkTarget::Kind::kAnchor);
}

TEST_CASE("a link to a missing file is reported with its resolved path", "[links]")
{
    Fixture f;
    const auto target = f.Resolve("missing.md");
    CHECK(target.kind == LinkTarget::Kind::kMissing);
    CHECK(target.path == f.docs / "guide" / "missing.md");
}

TEST_CASE("ToFileUrl percent-encodes what a URL cannot carry", "[links]")
{
    CHECK(DocumentLoader::ToFileUrl("/home/u/my docs/a#b.cpp")
          == "file:///home/u/my%20docs/a%23b.cpp");
}
