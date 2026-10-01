#pragma once

#include <filesystem>

#include "../core/code_model.h"

namespace dev_dash::services
{
    struct ExtractionResult
    {
        core::CodeModel model;
        int filesRead = 0;
        int filesWithSyntaxErrors = 0;  // parsed anyway: tree-sitter recovers around the error
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
        // .cxx), in path order. Classes keep the order in which they are
        // found; relations are sorted, one per pair of classes. A relation
        // may point to a name outside the model: a base class from an
        // external library, for example.
        ExtractionResult Extract(const std::filesystem::path& root) const;
    };
}
