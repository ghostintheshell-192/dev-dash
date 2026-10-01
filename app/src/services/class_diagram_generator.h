#pragma once

#include <set>
#include <string>

#include "../core/code_model.h"

namespace dev_dash::services
{
    // Colours written into the DOT text, as Graphviz colour strings ("#rrggbb"
    // or "#rrggbbaa"). The UI fills them from the theme; an empty string leaves
    // the attribute out, so the renderer's default applies.
    struct DiagramPalette
    {
        std::string background;
        std::string classFill;
        std::string classBorder;
        std::string text;
        std::string ghostBorder;
        std::string ghostText;
        std::string edge;
    };

    struct DiagramOptions
    {
        // Draw the direct neighbours of the selected classes as ghost nodes:
        // name only, no members.
        bool showNeighbours = true;
        // Least visible access shown in the class boxes: kPublic shows only the
        // public members, kPrivate shows all of them.
        core::MemberAccess memberAccess = core::MemberAccess::kPublic;
        DiagramPalette palette;
    };

    // Turns a code model and a selection of classes into the DOT text of a UML
    // class diagram (feature-code-graph). The selected classes are drawn as
    // boxes with their members, their direct neighbours as ghosts. Pure: the
    // same input always gives the same text, in the order of the model.
    class ClassDiagramGenerator
    {
    public:
        ClassDiagramGenerator() = default;

        std::string Generate(const core::CodeModel& model,
                             const std::set<std::string>& selection,
                             const DiagramOptions& options) const;
    };
}
