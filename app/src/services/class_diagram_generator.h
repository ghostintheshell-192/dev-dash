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
        // The access levels whose members the class boxes show, each on its
        // own: {kPublic} by default, {} shows no member, {kPrivate} only the
        // private ones.
        std::set<core::MemberAccess> shownAccess = {core::MemberAccess::kPublic};
        // Class boxes as records ({name|attributes|methods}), the UML
        // compartments. Off: plain boxes with the name and the members on
        // separate lines, for renderers without records (ImGuiDot, for now).
        bool recordShapes = true;
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
