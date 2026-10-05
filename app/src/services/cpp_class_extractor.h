#pragma once

#include <filesystem>
#include <optional>
#include <stop_token>
#include <string>
#include <vector>

#include "../core/code_model.h"

namespace dev_dash::services
{
    // A file with code the parser did not understand: macros it cannot
    // expand (TEST_CASE(...), SDLCALL), syntax the grammar misses (a default
    // argument "= {}"), or real errors. tree-sitter recovers around them, so
    // the rest of the file is read. The first such place is kept, for the
    // reader to tell which case it is.
    struct PartlyReadFile
    {
        std::filesystem::path file;
        int                   line = 0;   // 1-based
        std::string           text;       // that line, trimmed
    };

    struct ExtractionResult
    {
        core::CodeModel model;
        // The files read, in path order (the partly read ones too).
        std::vector<std::filesystem::path> filesRead;
        std::vector<PartlyReadFile> partlyReadFiles;
        bool cancelled = false;         // stopped before the end: the model is partial
        // Every directory holding C++ files, at any depth, relative to the
        // root and sorted: the excluded ones too, for the caller to offer
        // them back. The skipped ones (hidden, build trees) are not listed.
        std::vector<std::filesystem::path> sourceDirectories;
        // The directories left out of this reading, relative to the root:
        // the ones asked for, or those of the default rule.
        std::vector<std::filesystem::path> excludedDirectories;
    };

    // The base reader of the code graph (ADR-017): reads the C++ classes of a
    // source tree with tree-sitter and links the type names to the classes
    // with our own, simplified, C++ name lookup:
    //
    //   - qualified names are looked up as written (b::Node is b::Node);
    //   - lookup walks the enclosing scopes from the innermost outwards;
    //   - `using X = T;`, `typedef T X;` and alias templates are expanded;
    //   - `using ns::Name;` and `using namespace ns;` count in their scope;
    //   - template arguments are stripped from the base classes (CRTP).
    //
    // Out of reach, by design: macros (no preprocessor) and whatever needs the
    // compiler. A name that two using-directives make ambiguous gives an
    // uncertain relation (ADR-017 §2).
    //
    // Port of extract_ts2.py, the script of the extraction experiment
    // (.development/reference/technical/code-graph-extraction/). The common
    // extractor interface of ADR-017 §4 is, for now, the model it returns: a
    // virtual base comes with the second extractor (libclang), per ADR-010.
    class CppClassExtractor
    {
    public:
        CppClassExtractor() = default;

        // Reads every C++ source file under root (.h .hh .hpp .hxx .cpp .cc
        // .cxx), in path order, skipping the directories that hold no code
        // of the project: hidden ones, build trees (build, build-*,
        // cmake-build-*, out) and node_modules. Classes keep the order in
        // which they are found; relations are sorted, one per pair of
        // classes. A relation may point to a name outside the model: a base
        // class from an external library, for example.
        //
        // excludedDirectories (relative to root) leaves directories out, with
        // all they hold: they are still listed in sourceDirectories, not
        // read. Without it, the default rule applies: the directories named
        // test or tests stay out, as they hold the tests of the project, not
        // its design.
        //
        // Meant to run on a worker thread: a stop request is checked between
        // files, so that the caller can give up quickly (a project switch
        // during a long reading).
        ExtractionResult Extract(const std::filesystem::path& root,
                                 std::stop_token stop = {},
                                 const std::optional<std::vector<std::filesystem::path>>& excludedDirectories = {}) const;

        // The default rule: a directory named test or tests.
        static bool IsTestDirectory(const std::filesystem::path& directory);
    };
}
