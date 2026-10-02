#include <catch2/catch_test_macros.hpp>

#include "services/class_diagram_generator.h"

// The generator is a pure function: a model and a selection in, DOT text out.
// One whole expected output pins the format; the other tests check one rule
// each on the parts of the text it concerns.

using namespace dev_dash;
using core::MemberAccess;
using core::RelationKind;
using services::ClassDiagramGenerator;
using services::DiagramOptions;

namespace
{
    core::CodeMember Attribute(std::string name, std::string type,
                               MemberAccess access = MemberAccess::kPublic)
    {
        return core::CodeMember{std::move(name), std::move(type), access, false};
    }

    core::CodeMember Method(std::string name, std::string returnType,
                            MemberAccess access = MemberAccess::kPublic)
    {
        return core::CodeMember{std::move(name), std::move(returnType), access, true};
    }

    core::CodeClass Class(std::string name, std::vector<core::CodeMember> members = {})
    {
        return core::CodeClass{std::move(name), "src/file.h", std::move(members)};
    }

    core::CodeRelation Relation(std::string from, std::string to, RelationKind kind, bool certain = true)
    {
        return core::CodeRelation{std::move(from), std::move(to), kind, certain};
    }

    bool Contains(const std::string& text, const std::string& part)
    {
        return text.find(part) != std::string::npos;
    }

    std::string Generate(const core::CodeModel& model, const std::set<std::string>& selection,
                         const DiagramOptions& options = {})
    {
        return ClassDiagramGenerator().Generate(model, selection, options);
    }
}

TEST_CASE("a selected class and its neighbour give the expected DOT text", "[diagram]")
{
    core::CodeModel model;
    model.classes = {
        Class("app::Engine", {Attribute("speed", "float"), Method("Start", "bool")}),
        Class("app::Part"),
    };
    model.relations = {Relation("app::Engine", "app::Part", RelationKind::kComposes)};

    DiagramOptions options;
    options.palette.background  = "#181614";
    options.palette.classFill   = "#1f1d1a";
    options.palette.classBorder = "#e0a64e";
    options.palette.text        = "#d8d3c7";
    options.palette.ghostBorder = "#353026";
    options.palette.ghostText   = "#8a8478";
    options.palette.edge        = "#8a8478";

    CHECK(Generate(model, {"app::Engine"}, options) ==
          "digraph ClassDiagram\n"
          "{\n"
          "    graph [rankdir=TB, bgcolor=\"#181614\"];\n"
          "    node [shape=record, style=filled, fillcolor=\"#1f1d1a\", color=\"#e0a64e\", fontcolor=\"#d8d3c7\"];\n"
          "    edge [color=\"#8a8478\", fontcolor=\"#8a8478\"];\n"
          "\n"
          "    \"app::Engine\" [label=\"{Engine|+ speed : float\\l|+ Start() : bool\\l}\"];\n"
          "\n"
          "    \"app::Part\" [shape=box, style=\"rounded,dashed\", color=\"#353026\", fontcolor=\"#8a8478\", label=\"Part\"];\n"
          "\n"
          "    \"app::Engine\" -> \"app::Part\" [dir=back, arrowtail=diamond];\n"
          "}\n");
}

TEST_CASE("an empty palette leaves the colours to the renderer", "[diagram]")
{
    core::CodeModel model;
    model.classes = {Class("A")};

    const std::string dot = Generate(model, {"A"});
    CHECK(Contains(dot, "graph [rankdir=TB];"));
    CHECK(Contains(dot, "node [shape=record];"));
    CHECK_FALSE(Contains(dot, "edge ["));
    CHECK_FALSE(Contains(dot, "color"));
}

TEST_CASE("the class box shows the members, without the special ones", "[diagram]")
{
    core::CodeModel model;
    model.classes = {Class("ns::Panel", {
        Method("Panel", ""),
        Method("~Panel", ""),
        Method("operator=", "Panel"),
        Method("Render", "void"),
        Method("Find", "int"),
        Method("Find", "int"),          // overload: one line
        Attribute("_size", "int", MemberAccess::kPrivate),
        Method("Layout", "void", MemberAccess::kProtected),
    })};

    SECTION("public members by default")
    {
        CHECK(Contains(Generate(model, {"ns::Panel"}), "label=\"{Panel|+ Render()\\l+ Find() : int\\l}\""));
    }

    SECTION("all the members on request, with their access symbol")
    {
        DiagramOptions options;
        options.memberAccess = MemberAccess::kPrivate;
        CHECK(Contains(Generate(model, {"ns::Panel"}, options),
                       "label=\"{Panel|- _size : int\\l|+ Render()\\l+ Find() : int\\l# Layout()\\l}\""));
    }
}

TEST_CASE("without records the class is a box with one line per member", "[diagram]")
{
    core::CodeModel model;
    model.classes = {Class("ns::Panel", {Attribute("size", "std::vector<int>"), Method("Render", "void")}),
                     Class("ns::Empty")};

    DiagramOptions options;
    options.recordShapes = false;
    const std::string dot = Generate(model, {"ns::Panel", "ns::Empty"}, options);
    CHECK(Contains(dot, "node [shape=box];"));
    CHECK(Contains(dot, "label=\"Panel\\n\\n+ size : std::vector<int>\\l+ Render()\\l\""));
    CHECK(Contains(dot, "label=\"Empty\""));
}

TEST_CASE("record and quoting characters are escaped", "[diagram]")
{
    core::CodeModel model;
    model.classes = {Class("ns::Holder", {Attribute("items", "std::vector<std::pair<int, int>>"),
                                          Attribute("flags", "Set{a|b}"),
                                          Attribute("path", "C:\\dir \"x\"")})};

    const std::string dot = Generate(model, {"ns::Holder"});
    CHECK(Contains(dot, "+ items : std::vector\\<std::pair\\<int, int\\>\\>\\l"));
    CHECK(Contains(dot, "+ flags : Set\\{a\\|b\\}\\l"));
    CHECK(Contains(dot, "+ path : C:\\\\dir \\\"x\\\"\\l"));
}

TEST_CASE("the neighbours of the selection are ghosts, on request", "[diagram]")
{
    core::CodeModel model;
    model.classes = {Class("a::Owner"), Class("a::Used"), Class("a::User"), Class("a::Far")};
    model.relations = {
        Relation("a::Owner", "a::Used", RelationKind::kAggregates),
        Relation("a::User", "a::Owner", RelationKind::kDepends),
        Relation("a::Used", "a::Far", RelationKind::kComposes),   // two steps away
    };

    SECTION("shown by default, in the order of the model")
    {
        const std::string dot = Generate(model, {"a::Owner"});
        const std::size_t used = dot.find("\"a::Used\" [shape=box");
        const std::size_t user = dot.find("\"a::User\" [shape=box");
        REQUIRE(used != std::string::npos);
        REQUIRE(user != std::string::npos);
        CHECK(used < user);
        CHECK_FALSE(Contains(dot, "a::Far"));
        CHECK(Contains(dot, "\"a::Owner\" -> \"a::Used\""));
        CHECK(Contains(dot, "\"a::User\" -> \"a::Owner\""));
    }

    SECTION("hidden: only the relations inside the selection remain")
    {
        DiagramOptions options;
        options.showNeighbours = false;
        const std::string dot = Generate(model, {"a::Owner", "a::Used"}, options);
        CHECK(Contains(dot, "\"a::Owner\" -> \"a::Used\""));
        CHECK_FALSE(Contains(dot, "a::User"));
        CHECK_FALSE(Contains(dot, "shape=box"));
    }
}

TEST_CASE("each relation kind has its UML notation", "[diagram]")
{
    core::CodeModel model;
    model.classes = {Class("Base"), Class("Derived"), Class("Part"), Class("Peer"), Class("Arg")};
    model.relations = {
        Relation("Derived", "Base", RelationKind::kInherits),
        Relation("Derived", "Part", RelationKind::kComposes),
        Relation("Derived", "Peer", RelationKind::kAggregates),
        Relation("Derived", "Arg", RelationKind::kDepends),
    };

    const std::string dot = Generate(model, {"Derived"});
    // Written base -> derived, so that the layout puts the base above.
    CHECK(Contains(dot, "\"Base\" -> \"Derived\" [dir=back, arrowtail=onormal];"));
    CHECK(Contains(dot, "\"Derived\" -> \"Part\" [dir=back, arrowtail=diamond];"));
    CHECK(Contains(dot, "\"Derived\" -> \"Peer\" [dir=back, arrowtail=odiamond];"));
    CHECK(Contains(dot, "\"Derived\" -> \"Arg\" [style=dashed, arrowhead=vee];"));
}

TEST_CASE("an uncertain relation is marked, not guessed", "[diagram]")
{
    core::CodeModel model;
    model.classes = {Class("A"), Class("B"), Class("C")};
    model.relations = {
        Relation("A", "B", RelationKind::kComposes, false),
        Relation("A", "C", RelationKind::kDepends, false),
    };

    const std::string dot = Generate(model, {"A"});
    CHECK(Contains(dot, "\"A\" -> \"B\" [dir=back, arrowtail=diamond, style=dotted, label=\"?\"];"));
    CHECK(Contains(dot, "\"A\" -> \"C\" [arrowhead=vee, style=dotted, label=\"?\"];"));
}

TEST_CASE("the labels leave out the scope shared by the drawn classes", "[diagram]")
{
    core::CodeModel model;
    model.classes = {Class("dev_dash::ui::Panel"), Class("dev_dash::services::Loader"), Class("dev_dash::ui::Theme")};
    model.relations = {Relation("dev_dash::ui::Panel", "dev_dash::services::Loader", RelationKind::kAggregates)};

    SECTION("different scopes keep the part that tells them apart")
    {
        const std::string dot = Generate(model, {"dev_dash::ui::Panel"});
        CHECK(Contains(dot, "label=\"{ui::Panel}\""));
        CHECK(Contains(dot, "label=\"services::Loader\""));
    }

    SECTION("a single class keeps only its own name")
    {
        CHECK(Contains(Generate(model, {"dev_dash::ui::Theme"}), "label=\"{Theme}\""));
    }

    SECTION("a name at global scope does not keep the others in full")
    {
        model.relations.push_back(Relation("dev_dash::ui::Panel", "imgui_md", RelationKind::kInherits));
        const std::string dot = Generate(model, {"dev_dash::ui::Panel"});
        CHECK(Contains(dot, "label=\"{ui::Panel}\""));
        CHECK(Contains(dot, "label=\"services::Loader\""));
        CHECK(Contains(dot, "label=\"imgui_md\""));
    }
}

TEST_CASE("names outside the model are ignored in the selection", "[diagram]")
{
    core::CodeModel model;
    model.classes = {Class("A")};

    const std::string dot = Generate(model, {"Missing"});
    CHECK_FALSE(Contains(dot, "Missing"));
    CHECK_FALSE(Contains(dot, "\"A\""));
}
