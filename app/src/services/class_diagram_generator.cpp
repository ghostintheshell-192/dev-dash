#include "class_diagram_generator.h"

#include <algorithm>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace dev_dash::services
{
    namespace
    {
        constexpr std::string_view kScope = "::";

        // A DOT quoted string. Inside quotes the double quote is the only
        // character to escape; backslashes pass through, so that the escapes of
        // a label (\l, \{) reach Graphviz.
        std::string Quote(std::string_view text)
        {
            std::string out = "\"";
            for (const char c : text)
            {
                if (c == '"')
                    out += '\\';
                out += c;
            }
            out += '"';
            return out;
        }

        // Text of a label, where a backslash starts an escape: a literal one is
        // doubled.
        std::string EscapeLabelText(std::string_view text)
        {
            std::string out;
            for (const char c : text)
            {
                if (c == '\\')
                    out += '\\';
                out += c;
            }
            return out;
        }

        // Text of a record field: the characters that structure a record label
        // ({ } | < >) are escaped too.
        std::string EscapeRecordText(std::string_view text)
        {
            std::string out;
            for (const char c : text)
            {
                if (std::string_view("{}|<>\\").find(c) != std::string_view::npos)
                    out += '\\';
                out += c;
            }
            return out;
        }

        // Appends `name=value` to an attribute list; an empty value leaves the
        // attribute out.
        void AppendAttribute(std::string& list, std::string_view name, std::string_view value,
                             bool quote = true)
        {
            if (value.empty())
                return;
            if (!list.empty())
                list += ", ";
            list += name;
            list += '=';
            list += quote ? Quote(value) : std::string(value);
        }

        std::string ShortName(std::string_view qualifiedName)
        {
            const std::size_t pos = qualifiedName.rfind(kScope);
            return std::string(pos == std::string_view::npos ? qualifiedName
                                                             : qualifiedName.substr(pos + kScope.size()));
        }

        // The namespace prefix ("a::b::") shared by all the scoped names, so
        // that the labels can leave it out. It never covers the last component
        // of a name: a lone class keeps its own name. Names at global scope
        // (often from an external library, like a base class) do not take
        // part, or a single one of them would keep every label in full.
        std::string CommonScopePrefix(const std::vector<std::string>& allNames)
        {
            std::vector<std::string> names;
            for (const std::string& name : allNames)
                if (name.find(kScope) != std::string::npos)
                    names.push_back(name);
            if (names.empty())
                return {};

            // Start from the scope of the first name, then shorten it until it
            // fits every name.
            const std::size_t lastScope = names.front().rfind(kScope);
            std::string prefix = lastScope == std::string::npos ? std::string()
                                                                : names.front().substr(0, lastScope + kScope.size());
            for (const std::string& name : names)
            {
                while (!prefix.empty() && !(name.starts_with(prefix) && name.size() > prefix.size()))
                {
                    // Drop the last component of the prefix: "a::b::" -> "a::".
                    if (prefix.size() <= kScope.size())
                    {
                        prefix.clear();
                        break;
                    }
                    const std::size_t pos = prefix.rfind(kScope, prefix.size() - kScope.size() - 1);
                    prefix.resize(pos == std::string::npos ? 0 : pos + kScope.size());
                }
            }
            return prefix;
        }

        int Visibility(core::MemberAccess access)
        {
            switch (access)
            {
            case core::MemberAccess::kPublic:    return 0;
            case core::MemberAccess::kProtected: return 1;
            case core::MemberAccess::kPrivate:   return 2;
            }
            return 2;
        }

        char AccessSymbol(core::MemberAccess access)
        {
            switch (access)
            {
            case core::MemberAccess::kPublic:    return '+';
            case core::MemberAccess::kProtected: return '#';
            case core::MemberAccess::kPrivate:   return '-';
            }
            return '-';
        }

        // Constructors, destructor and operators say little in a class
        // diagram and would crowd every box.
        bool IsSpecialMember(const core::CodeMember& member, std::string_view className)
        {
            return member.name == className || member.name.starts_with('~')
                || member.name.starts_with("operator");
        }

        // One record field: the lines, each left-justified ("\l").
        std::string RecordField(const std::vector<std::string>& lines)
        {
            std::string field;
            for (const std::string& line : lines)
                field += EscapeRecordText(line) + "\\l";
            return field;
        }

        std::string ClassLabel(const core::CodeClass& cls, std::string_view displayName,
                               core::MemberAccess memberAccess, bool recordShape)
        {
            const std::string className = ShortName(cls.qualifiedName);
            std::vector<std::string> attributes;
            std::vector<std::string> methods;

            for (const core::CodeMember& member : cls.members)
            {
                if (Visibility(member.access) > Visibility(memberAccess) || IsSpecialMember(member, className))
                    continue;

                std::string line = std::string(1, AccessSymbol(member.access)) + ' ' + member.name;
                if (member.isMethod)
                {
                    line += "()";
                    if (!member.type.empty() && member.type != "void")
                        line += " : " + member.type;
                    // Overloads differ only in the parameters, which the
                    // diagram does not show: one line is enough.
                    if (std::find(methods.begin(), methods.end(), line) == methods.end())
                        methods.push_back(std::move(line));
                }
                else
                {
                    if (!member.type.empty())
                        line += " : " + member.type;
                    attributes.push_back(std::move(line));
                }
            }

            if (!recordShape)
            {
                // Plain box: the name centred ("\n"), an empty line that sets
                // it apart as a heading (records draw a separator instead),
                // then one left-justified line ("\l") per member, attributes
                // first.
                std::string label = EscapeLabelText(displayName);
                if (attributes.empty() && methods.empty())
                    return label;
                label += "\\n\\n";
                for (const std::string& line : attributes)
                    label += EscapeLabelText(line) + "\\l";
                for (const std::string& line : methods)
                    label += EscapeLabelText(line) + "\\l";
                return label;
            }

            std::string label = "{" + EscapeRecordText(displayName);
            if (!attributes.empty())
                label += "|" + RecordField(attributes);
            if (!methods.empty())
                label += "|" + RecordField(methods);
            label += "}";
            return label;
        }

        // Edge attributes in UML notation. Inheritance is written base -> derived
        // (see Generate) so that the layout puts the base above; the triangle
        // then sits at the tail, on the base.
        std::string EdgeAttributes(const core::CodeRelation& relation)
        {
            std::string attributes;
            switch (relation.kind)
            {
            case core::RelationKind::kInherits:
                attributes = "dir=back, arrowtail=onormal";
                break;
            case core::RelationKind::kComposes:
                attributes = "dir=back, arrowtail=diamond";
                break;
            case core::RelationKind::kAggregates:
                attributes = "dir=back, arrowtail=odiamond";
                break;
            case core::RelationKind::kDepends:
                attributes = "style=dashed, arrowhead=vee";
                break;
            }

            // An uncertain relation is shown as such instead of guessed
            // (ADR-017 §2): dotted line and a question mark.
            if (!relation.certain)
            {
                if (relation.kind == core::RelationKind::kDepends)
                    attributes = "arrowhead=vee";
                attributes += ", style=dotted, label=\"?\"";
            }
            return attributes;
        }
    }

    std::string ClassDiagramGenerator::Generate(const core::CodeModel& model,
                                                const std::set<std::string>& selection,
                                                const DiagramOptions& options) const
    {
        // ----- What to draw

        std::unordered_map<std::string_view, const core::CodeClass*> classesByName;
        for (const core::CodeClass& cls : model.classes)
            classesByName.emplace(cls.qualifiedName, &cls);

        std::vector<const core::CodeClass*> selected;
        for (const core::CodeClass& cls : model.classes)
            if (selection.contains(cls.qualifiedName))
                selected.push_back(&cls);

        const auto isSelected = [&](const std::string& name)
        { return selection.contains(name) && classesByName.contains(name); };

        std::vector<const core::CodeRelation*> relations;
        std::set<std::string> ghostNames;
        for (const core::CodeRelation& relation : model.relations)
        {
            const bool fromSelected = isSelected(relation.from);
            const bool toSelected   = isSelected(relation.to);
            if (!(fromSelected && toSelected) && !(options.showNeighbours && (fromSelected || toSelected)))
                continue;

            relations.push_back(&relation);
            if (!fromSelected)
                ghostNames.insert(relation.from);
            if (!toSelected)
                ghostNames.insert(relation.to);
        }

        // Ghosts in the order of the model; names the model does not know come
        // last, sorted.
        std::vector<std::string> ghosts;
        for (const core::CodeClass& cls : model.classes)
            if (ghostNames.erase(cls.qualifiedName) > 0)
                ghosts.push_back(cls.qualifiedName);
        ghosts.insert(ghosts.end(), ghostNames.begin(), ghostNames.end());

        std::vector<std::string> drawnNames = ghosts;
        for (const core::CodeClass* cls : selected)
            drawnNames.push_back(cls->qualifiedName);
        const std::string prefix = CommonScopePrefix(drawnNames);
        const auto displayName = [&](const std::string& name)
        { return name.starts_with(prefix) ? name.substr(prefix.size()) : name; };

        // ----- DOT text

        const DiagramPalette& palette = options.palette;
        std::string dot = "digraph ClassDiagram\n{\n";

        std::string graphAttributes = "rankdir=TB";
        AppendAttribute(graphAttributes, "bgcolor", palette.background);
        dot += "    graph [" + graphAttributes + "];\n";

        std::string nodeAttributes = options.recordShapes ? "shape=record" : "shape=box";
        if (!palette.classFill.empty())
        {
            AppendAttribute(nodeAttributes, "style", "filled", false);
            AppendAttribute(nodeAttributes, "fillcolor", palette.classFill);
        }
        AppendAttribute(nodeAttributes, "color", palette.classBorder);
        AppendAttribute(nodeAttributes, "fontcolor", palette.text);
        dot += "    node [" + nodeAttributes + "];\n";

        std::string edgeAttributes;
        AppendAttribute(edgeAttributes, "color", palette.edge);
        AppendAttribute(edgeAttributes, "fontcolor", palette.edge);
        if (!edgeAttributes.empty())
            dot += "    edge [" + edgeAttributes + "];\n";

        if (!selected.empty())
            dot += "\n";
        for (const core::CodeClass* cls : selected)
            dot += "    " + Quote(cls->qualifiedName) + " [label="
                 + Quote(ClassLabel(*cls, displayName(cls->qualifiedName), options.memberAccess,
                                    options.recordShapes)) + "];\n";

        if (!ghosts.empty())
            dot += "\n";
        for (const std::string& name : ghosts)
        {
            std::string attributes = "shape=box, style=\"rounded,dashed\"";
            AppendAttribute(attributes, "color", palette.ghostBorder);
            AppendAttribute(attributes, "fontcolor", palette.ghostText);
            AppendAttribute(attributes, "label", EscapeLabelText(displayName(name)));
            dot += "    " + Quote(name) + " [" + attributes + "];\n";
        }

        if (!relations.empty())
            dot += "\n";
        for (const core::CodeRelation* relation : relations)
        {
            const bool inherits = relation->kind == core::RelationKind::kInherits;
            const std::string& tail = inherits ? relation->to : relation->from;
            const std::string& head = inherits ? relation->from : relation->to;
            dot += "    " + Quote(tail) + " -> " + Quote(head) + " [" + EdgeAttributes(*relation) + "];\n";
        }

        dot += "}\n";
        return dot;
    }
}
