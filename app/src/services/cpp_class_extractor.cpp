#include "cpp_class_extractor.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <tree_sitter/api.h>

extern "C" const TSLanguage* tree_sitter_cpp();

namespace dev_dash::services
{
    namespace
    {
        constexpr std::string_view kScope = "::";

        // Wrappers that own a smart-pointer-like or container type: a member of
        // such a type owns what it names, even when the declarator has a '*'.
        constexpr std::array<std::string_view, 10> kOwningWrappers = {
            "unique_ptr", "optional", "vector", "array", "deque",
            "list", "map", "unordered_map", "set", "shared_ptr",
        };

        constexpr std::array<std::string_view, 7> kSourceExtensions = {
            ".h", ".hh", ".hpp", ".hxx", ".cpp", ".cc", ".cxx",
        };

        // Lookup gives up past this depth: aliases of aliases, or a cycle.
        constexpr int kMaxLookupDepth = 8;

        // ----- tree-sitter helpers

        struct TreeDeleter
        {
            void operator()(TSTree* tree) const { ts_tree_delete(tree); }
        };
        using TreePtr = std::unique_ptr<TSTree, TreeDeleter>;

        struct ParserDeleter
        {
            void operator()(TSParser* parser) const { ts_parser_delete(parser); }
        };
        using ParserPtr = std::unique_ptr<TSParser, ParserDeleter>;

        std::string_view Type(TSNode node)
        {
            return ts_node_type(node);
        }

        std::optional<TSNode> Field(TSNode node, std::string_view name)
        {
            const TSNode child = ts_node_child_by_field_name(node, name.data(),
                                                             static_cast<std::uint32_t>(name.size()));
            if (ts_node_is_null(child))
                return std::nullopt;
            return child;
        }

        std::vector<TSNode> Children(TSNode node)
        {
            std::vector<TSNode> children;
            const std::uint32_t count = ts_node_child_count(node);
            for (std::uint32_t i = 0; i < count; ++i)
                children.push_back(ts_node_child(node, i));
            return children;
        }

        std::vector<TSNode> NamedChildren(TSNode node)
        {
            std::vector<TSNode> children;
            const std::uint32_t count = ts_node_named_child_count(node);
            for (std::uint32_t i = 0; i < count; ++i)
                children.push_back(ts_node_named_child(node, i));
            return children;
        }

        std::vector<TSNode> FieldChildren(TSNode node, std::string_view name)
        {
            std::vector<TSNode> children;
            const std::uint32_t count = ts_node_child_count(node);
            for (std::uint32_t i = 0; i < count; ++i)
            {
                const char* field = ts_node_field_name_for_child(node, i);
                if (field != nullptr && name == field)
                    children.push_back(ts_node_child(node, i));
            }
            return children;
        }

        // ----- Names

        std::string Join(std::string_view scope, std::string_view name)
        {
            if (scope.empty())
                return std::string(name);
            return std::string(scope) + std::string(kScope) + std::string(name);
        }

        std::vector<std::string> SplitScope(std::string_view name)
        {
            std::vector<std::string> parts;
            std::size_t start = 0;
            while (true)
            {
                const std::size_t pos = name.find(kScope, start);
                parts.emplace_back(name.substr(start, pos - start));
                if (pos == std::string_view::npos)
                    break;
                start = pos + kScope.size();
            }
            return parts;
        }

        // The scope itself, then each enclosing scope, ending with the global
        // one ("").
        std::vector<std::string> EnclosingScopes(std::string_view scope)
        {
            std::vector<std::string> scopes;
            if (scope.empty())
            {
                scopes.emplace_back();
                return scopes;
            }
            const std::vector<std::string> parts = SplitScope(scope);
            for (std::size_t count = parts.size() + 1; count-- > 0;)
            {
                std::string joined;
                for (std::size_t i = 0; i < count; ++i)
                    joined = Join(joined, parts[i]);
                scopes.push_back(std::move(joined));
            }
            return scopes;
        }

        std::string LastComponent(std::string_view name)
        {
            const std::size_t pos = name.rfind(kScope);
            return std::string(pos == std::string_view::npos ? name : name.substr(pos + kScope.size()));
        }

        bool IsWordChar(char c)
        {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
        }

        // The (possibly qualified) names written in a piece of code:
        // "std::vector<a::Node>" -> {"std::vector", "a::Node"}.
        std::vector<std::string> NamesIn(std::string_view code)
        {
            std::vector<std::string> names;
            std::size_t i = 0;
            while (i < code.size())
            {
                if (!IsWordChar(code[i]))
                {
                    ++i;
                    continue;
                }
                std::string name;
                while (true)
                {
                    while (i < code.size() && IsWordChar(code[i]))
                        name += code[i++];
                    if (code.substr(i, kScope.size()) == kScope && i + kScope.size() < code.size()
                        && IsWordChar(code[i + kScope.size()]))
                    {
                        name += kScope;
                        i += kScope.size();
                        continue;
                    }
                    break;
                }
                names.push_back(std::move(name));
            }
            return names;
        }

        int Strength(core::RelationKind kind)
        {
            switch (kind)
            {
            case core::RelationKind::kInherits:   return 0;
            case core::RelationKind::kComposes:   return 1;
            case core::RelationKind::kAggregates: return 2;
            case core::RelationKind::kDepends:    return 3;
            }
            return 3;
        }

        // ----- The index name lookup works on

        struct SourceFile
        {
            std::filesystem::path path;
            std::string text;
            TreePtr tree;
        };

        struct ClassEntry
        {
            core::CodeClass info;
            TSNode node;
            const SourceFile* source;
            bool isStruct;
        };

        struct AliasEntry
        {
            std::string body;                       // the aliased type, as written
            std::vector<std::string> templateParameters;
            std::string scope;                      // where the alias is declared
        };

        struct UsingDeclaration
        {
            std::string target;                     // "a::Node"
            std::string scope;
        };

        struct LookupResult
        {
            bool isClass;                           // otherwise an alias
            std::string name;                       // qualified name found
            bool certain;
        };

        class Index
        {
        public:
            std::vector<ClassEntry> classes;
            std::unordered_map<std::string, std::size_t> classPositions;
            std::unordered_map<std::string, AliasEntry> aliases;
            std::unordered_map<std::string, std::map<std::string, UsingDeclaration>> usingDeclarations;
            std::unordered_map<std::string, std::vector<std::string>> usingDirectives;

            // Collects classes, aliases and using declarations/directives
            // below node, which sits in scope.
            void Collect(TSNode node, const std::string& scope, const SourceFile& source,
                         const std::vector<std::string>& templateParameters = {});

            // Resolves a (possibly qualified) name seen in scope.
            std::optional<LookupResult> Lookup(const std::string& name, const std::string& scope,
                                               int depth = 0) const;

            // The project classes named in a piece of code (a type), each
            // with its certainty, and whether an owning wrapper appears.
            void Mentions(std::string_view code, const std::string& scope,
                          std::map<std::string, bool>& found, bool& owning, int depth = 0) const;

        private:
            std::optional<LookupResult> Find(const std::string& name) const;
        };

        std::string Text(TSNode node, const SourceFile& source)
        {
            const std::uint32_t start = ts_node_start_byte(node);
            const std::uint32_t end   = ts_node_end_byte(node);
            return source.text.substr(start, end - start);
        }

        std::string Trim(std::string_view text, std::string_view characters)
        {
            const std::size_t first = text.find_first_not_of(characters);
            if (first == std::string_view::npos)
                return {};
            const std::size_t last = text.find_last_not_of(characters);
            return std::string(text.substr(first, last - first + 1));
        }

        void Index::Collect(TSNode node, const std::string& scope, const SourceFile& source,
                            const std::vector<std::string>& templateParameters)
        {
            for (const TSNode child : Children(node))
            {
                const std::string_view type = Type(child);

                if (type == "namespace_definition")
                {
                    const std::optional<TSNode> name = Field(child, "name");
                    if (const std::optional<TSNode> body = Field(child, "body"))
                        Collect(*body, name ? Join(scope, Text(*name, source)) : scope, source);
                }
                else if ((type == "class_specifier" || type == "struct_specifier") && Field(child, "body"))
                {
                    const std::optional<TSNode> name = Field(child, "name");
                    if (!name)
                        continue;
                    const std::string qualified = Join(scope, Text(*name, source));
                    if (!classPositions.contains(qualified))
                    {
                        classPositions.emplace(qualified, classes.size());
                        classes.push_back(ClassEntry{core::CodeClass{qualified, source.path, {}}, child, &source,
                                                     type == "struct_specifier"});
                    }
                    Collect(*Field(child, "body"), qualified, source);
                }
                else if (type == "template_declaration")
                {
                    std::vector<std::string> parameters;
                    if (const std::optional<TSNode> list = Field(child, "parameters"))
                    {
                        for (const TSNode parameter : NamedChildren(*list))
                        {
                            const std::optional<TSNode> name = Field(parameter, "name");
                            const std::uint32_t namedCount = ts_node_named_child_count(parameter);
                            if (name)
                                parameters.push_back(Text(*name, source));
                            else if (namedCount > 0)
                                parameters.push_back(Text(ts_node_named_child(parameter, namedCount - 1), source));
                        }
                    }
                    Collect(child, scope, source, parameters);
                }
                else if (type == "alias_declaration")
                {
                    const std::optional<TSNode> name = Field(child, "name");
                    const std::optional<TSNode> body = Field(child, "type");
                    if (name && body)
                        aliases[Join(scope, Text(*name, source))] =
                            AliasEntry{Text(*body, source), templateParameters, scope};
                }
                else if (type == "type_definition")
                {
                    const std::optional<TSNode> body = Field(child, "type");
                    if (!body)
                        continue;
                    for (const TSNode declarator : FieldChildren(child, "declarator"))
                        aliases[Join(scope, Text(declarator, source))] = AliasEntry{Text(*body, source), {}, scope};
                }
                else if (type == "using_declaration")
                {
                    constexpr std::string_view kUsingNamespace = "using namespace";
                    constexpr std::string_view kUsing          = "using";
                    const std::string raw = Text(child, source);
                    if (raw.starts_with(kUsingNamespace))
                        usingDirectives[scope].push_back(Trim(raw.substr(kUsingNamespace.size()), " ;"));
                    else
                    {
                        const std::string target = Trim(raw.substr(kUsing.size()), " ;");
                        usingDeclarations[scope][LastComponent(target)] = UsingDeclaration{target, scope};
                    }
                }
                else if (type == "declaration" || type == "field_declaration" || type == "declaration_list"
                         || type == "linkage_specification" || type == "field_declaration_list")
                {
                    Collect(child, scope, source, templateParameters);
                }
            }
        }

        std::optional<LookupResult> Index::Find(const std::string& name) const
        {
            if (classPositions.contains(name))
                return LookupResult{true, name, true};
            if (aliases.contains(name))
                return LookupResult{false, name, true};
            return std::nullopt;
        }

        std::optional<LookupResult> Index::Lookup(const std::string& name, const std::string& scope,
                                                  int depth) const
        {
            if (depth > kMaxLookupDepth)
                return std::nullopt;

            const std::size_t separator = name.find(kScope);
            const std::string first = name.substr(0, separator);
            const std::string rest  = separator == std::string::npos ? "" : name.substr(separator + kScope.size());

            for (const std::string& enclosing : EnclosingScopes(scope))
            {
                if (std::optional<LookupResult> found = Find(Join(enclosing, name)))
                    return found;

                // A using-declaration of the first component, in this scope.
                if (const auto declarations = usingDeclarations.find(enclosing);
                    declarations != usingDeclarations.end())
                {
                    if (const auto declaration = declarations->second.find(first);
                        declaration != declarations->second.end())
                    {
                        const std::string target =
                            declaration->second.target + (rest.empty() ? "" : std::string(kScope) + rest);
                        return Lookup(target, declaration->second.scope, depth + 1);
                    }
                }

                // Using-directives: look inside each namespace they nominate.
                // More than one hit makes the name ambiguous: the first one is
                // kept, marked uncertain.
                if (const auto directives = usingDirectives.find(enclosing); directives != usingDirectives.end())
                {
                    std::vector<LookupResult> hits;
                    for (const std::string& nominated : directives->second)
                    {
                        for (const std::string& candidate :
                             {Join(nominated, name), Join(Join(enclosing, nominated), name)})
                        {
                            const std::optional<LookupResult> found = Find(candidate);
                            if (found && std::none_of(hits.begin(), hits.end(), [&](const LookupResult& hit)
                                                      { return hit.name == found->name; }))
                                hits.push_back(*found);
                        }
                    }
                    if (!hits.empty())
                    {
                        hits.front().certain = hits.size() == 1;
                        return hits.front();
                    }
                }
            }
            return std::nullopt;
        }

        void Index::Mentions(std::string_view code, const std::string& scope,
                             std::map<std::string, bool>& found, bool& owning, int depth) const
        {
            if (depth > kMaxLookupDepth)
                return;

            for (const std::string& name : NamesIn(code))
            {
                if (std::find(kOwningWrappers.begin(), kOwningWrappers.end(), LastComponent(name))
                    != kOwningWrappers.end())
                {
                    owning = true;
                    continue;
                }

                const std::optional<LookupResult> hit = Lookup(name, scope);
                if (!hit)
                    continue;

                if (hit->isClass)
                {
                    // Certain if any mention of the class is.
                    found[hit->name] = found[hit->name] || hit->certain;
                    continue;
                }

                // An alias: what it names, without its template parameters
                // (placeholders, not types).
                const AliasEntry& alias = aliases.at(hit->name);
                std::string cleaned;
                for (const std::string& part : NamesIn(alias.body))
                    if (std::find(alias.templateParameters.begin(), alias.templateParameters.end(), part)
                        == alias.templateParameters.end())
                        cleaned += part + " ";

                std::map<std::string, bool> inner;
                Mentions(cleaned, alias.scope, inner, owning, depth + 1);
                for (const auto& [className, certain] : inner)
                    found[className] = found[className] || (certain && hit->certain);
            }
        }

        // ----- Declarators

        std::optional<TSNode> Inner(TSNode declarator)
        {
            if (std::optional<TSNode> next = Field(declarator, "declarator"))
                return next;
            if (Type(declarator) == "reference_declarator" && ts_node_named_child_count(declarator) > 0)
                return ts_node_named_child(declarator, 0);
            return std::nullopt;
        }

        bool IsFunction(TSNode declarator)
        {
            for (std::optional<TSNode> current = declarator; current; current = Inner(*current))
                if (Type(*current) == "function_declarator")
                    return true;
            return false;
        }

        std::string DeclaratorName(TSNode declarator, const SourceFile& source)
        {
            constexpr std::array<std::string_view, 5> kNameTypes = {
                "identifier", "field_identifier", "destructor_name", "operator_name", "qualified_identifier",
            };

            std::optional<TSNode> current = declarator;
            while (current && std::find(kNameTypes.begin(), kNameTypes.end(), Type(*current)) == kNameTypes.end())
            {
                std::optional<TSNode> next = Inner(*current);
                if (!next && ts_node_named_child_count(*current) > 0)
                    next = ts_node_named_child(*current, 0);
                current = next;
            }
            return current ? Text(*current, source) : "?";
        }

        // ----- Relations

        struct RelationKey
        {
            std::string from;
            std::string to;
            auto operator<=>(const RelationKey&) const = default;
        };

        // Keeps one relation per pair of classes: the strongest kind, and
        // between equal kinds the certain one.
        void AddRelation(std::map<RelationKey, core::CodeRelation>& relations, core::CodeRelation relation)
        {
            const RelationKey key{relation.from, relation.to};
            const auto existing = relations.find(key);
            if (existing == relations.end())
            {
                relations.emplace(key, std::move(relation));
                return;
            }
            core::CodeRelation& kept = existing->second;
            if (Strength(relation.kind) < Strength(kept.kind)
                || (relation.kind == kept.kind && relation.certain && !kept.certain))
                kept = std::move(relation);
        }

        void ReadClass(ClassEntry& entry, const Index& index, std::map<RelationKey, core::CodeRelation>& relations)
        {
            const SourceFile& source = *entry.source;
            const std::string& qualified = entry.info.qualifiedName;

            // ----- Base classes

            for (const TSNode child : Children(entry.node))
            {
                if (Type(child) != "base_class_clause")
                    continue;
                for (const TSNode base : NamedChildren(child))
                {
                    const std::string_view type = Type(base);
                    if (type != "type_identifier" && type != "qualified_identifier" && type != "template_type")
                        continue;
                    // CRTP: Base<Derived> -> Base.
                    const std::string raw  = Text(base, source);
                    const std::string bare = raw.substr(0, raw.find('<'));
                    const std::optional<LookupResult> hit = index.Lookup(bare, qualified);
                    AddRelation(relations, core::CodeRelation{qualified, hit ? hit->name : bare,
                                                              core::RelationKind::kInherits,
                                                              hit ? hit->certain : true});
                }
            }

            // ----- Members

            core::MemberAccess access = entry.isStruct ? core::MemberAccess::kPublic : core::MemberAccess::kPrivate;
            for (const TSNode child : Children(*Field(entry.node, "body")))
            {
                const std::string_view type = Type(child);
                if (type == "access_specifier")
                {
                    const std::string specifier = Trim(Text(child, source), " :");
                    access = specifier == "public"      ? core::MemberAccess::kPublic
                           : specifier == "protected" ? core::MemberAccess::kProtected
                                                        : core::MemberAccess::kPrivate;
                    continue;
                }
                if (type != "field_declaration" && type != "declaration" && type != "function_definition")
                    continue;

                const std::optional<TSNode> declarator = Field(child, "declarator");
                if (!declarator)
                    continue;
                const std::optional<TSNode> typeNode = Field(child, "type");
                const std::string typeText       = typeNode ? Text(*typeNode, source) : "";
                const std::string declaratorText = Text(*declarator, source);
                const bool isMethod              = IsFunction(*declarator);

                entry.info.members.push_back(
                    core::CodeMember{DeclaratorName(*declarator, source), typeText, access, isMethod});

                std::map<std::string, bool> mentioned;
                bool owning = false;
                if (isMethod)
                {
                    // A type in the signature: a dependency.
                    index.Mentions(typeText + " " + declaratorText, qualified, mentioned, owning);
                    for (const auto& [target, certain] : mentioned)
                        if (target != qualified)
                            AddRelation(relations, core::CodeRelation{qualified, target,
                                                                      core::RelationKind::kDepends, certain});
                    continue;
                }

                // An attribute: owned unless held through a reference or a
                // plain pointer.
                index.Mentions(typeText, qualified, mentioned, owning);
                const bool reference = declaratorText.find('&') != std::string::npos
                                    || (declaratorText.find('*') != std::string::npos && !owning);
                const core::RelationKind kind =
                    reference && !owning ? core::RelationKind::kAggregates : core::RelationKind::kComposes;
                for (const auto& [target, certain] : mentioned)
                    if (target != qualified)
                        AddRelation(relations, core::CodeRelation{qualified, target, kind, certain});
            }
        }

        // Directories that hold no code of the project: hidden ones (.git,
        // .cache...), build trees (where CPM and FetchContent also keep the
        // sources of the dependencies) and node_modules.
        bool IsSkippedDirectory(const std::filesystem::path& path)
        {
            const std::string name = path.filename().string();
            return name.starts_with('.') || name == "build" || name.starts_with("build-")
                || name.starts_with("cmake-build-") || name == "out" || name == "node_modules";
        }

        bool IsSourceFile(const std::filesystem::path& path)
        {
            const std::string extension = path.extension().string();
            return std::find(kSourceExtensions.begin(), kSourceExtensions.end(), extension)
                != kSourceExtensions.end();
        }

        std::string ReadFile(const std::filesystem::path& path)
        {
            std::ifstream in(path, std::ios::binary);
            return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        }
    }

    ExtractionResult CppClassExtractor::Extract(const std::filesystem::path& root, std::stop_token stop) const
    {
        ExtractionResult result;

        std::vector<std::filesystem::path> paths;
        std::error_code ec;
        for (auto it = std::filesystem::recursive_directory_iterator(
                 root, std::filesystem::directory_options::skip_permission_denied, ec);
             !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec))
        {
            if (it->is_directory(ec) && IsSkippedDirectory(it->path()))
                it.disable_recursion_pending();
            else if (it->is_regular_file(ec) && IsSourceFile(it->path()))
                paths.push_back(it->path());
        }
        std::sort(paths.begin(), paths.end());

        // ----- Parse every file and index the names. The files stay alive:
        // the class entries point into their trees and texts.

        const ParserPtr parser(ts_parser_new());
        ts_parser_set_language(parser.get(), tree_sitter_cpp());

        std::vector<std::unique_ptr<SourceFile>> files;
        Index index;
        for (const std::filesystem::path& path : paths)
        {
            if (stop.stop_requested())
            {
                result.cancelled = true;
                return result;
            }

            auto file  = std::make_unique<SourceFile>();
            file->path = path;
            file->text = ReadFile(path);
            file->tree.reset(ts_parser_parse_string(parser.get(), nullptr, file->text.data(),
                                                    static_cast<std::uint32_t>(file->text.size())));
            if (!file->tree)
                continue;

            const TSNode rootNode = ts_tree_root_node(file->tree.get());
            ++result.filesRead;
            if (ts_node_has_error(rootNode))
                result.partlyReadFiles.push_back(path);
            index.Collect(rootNode, "", *file);
            files.push_back(std::move(file));
        }

        // ----- Read the members and the relations of each class

        std::map<RelationKey, core::CodeRelation> relations;
        for (ClassEntry& entry : index.classes)
            ReadClass(entry, index, relations);

        for (ClassEntry& entry : index.classes)
            result.model.classes.push_back(std::move(entry.info));
        for (auto& [key, relation] : relations)
            result.model.relations.push_back(std::move(relation));
        return result;
    }
}
