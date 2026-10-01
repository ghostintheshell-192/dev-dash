#pragma once

#include <string>

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

    class StatusSink;

    // Explains the last item in the status bar while the mouse is over it,
    // even when the item is disabled. The app's replacement for tooltips.
    void StatusHint(StatusSink& status, std::string text);

    // A dim "(?)" that explains the control next to it in the status bar.
    void HelpMarker(StatusSink& status, const char* text);
}
