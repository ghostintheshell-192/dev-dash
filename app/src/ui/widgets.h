#pragma once

#include <imgui.h>

namespace dev_dash::ui
{
    // Pill-shaped status badge: rounded tinted background, colored label.
    // One visual language for statuses across views (diff kinds, snapshot
    // kinds, layer provenance...).
    void StatusBadge(const char* label, const ImVec4& color);

    // A quiet text button: transparent until hovered. For secondary row
    // actions that should be discoverable without shouting.
    bool GhostButton(const char* label);

    // Canonical ImGui help affordance: a dim "(?)" that explains itself in
    // a wrapped tooltip on hover. Place next to the control it explains.
    void HelpMarker(const char* text);
}
