#pragma once

#include <filesystem>
#include <string>
#include <vector>

// The structure of a code base as the code graph sees it (feature-code-graph):
// classes, their members, and the relations between classes. Every extractor
// produces this model, whatever its source (ADR-017 §4).

namespace dev_dash::core
{
    enum class MemberAccess
    {
        kPublic,
        kProtected,
        kPrivate,
    };

    struct CodeMember
    {
        std::string name;
        std::string type;       // attribute type, or method return type ("" for constructors)
        MemberAccess access;
        bool isMethod;
    };

    struct CodeClass
    {
        std::string qualifiedName;      // "ns::Outer::Name", the identity of the class
        std::filesystem::path file;     // where the class is defined
        std::vector<CodeMember> members;
    };

    enum class RelationKind
    {
        kInherits,      // from derives from to
        kComposes,      // from owns a to: by value, unique_ptr, optional, container of
        kAggregates,    // from refers to a to it does not own: reference or plain pointer
        kDepends,       // to appears in a method signature of from, not as a member
    };

    struct CodeRelation
    {
        std::string from;       // qualified names
        std::string to;
        RelationKind kind;
        bool certain;           // false when the extractor could not resolve the name for sure (ADR-017 §2)
    };

    struct CodeModel
    {
        std::vector<CodeClass> classes;
        std::vector<CodeRelation> relations;
    };
}
