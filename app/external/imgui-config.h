// Compile-time configuration for Dear ImGui.
// Pulled in via the IMGUI_USER_CONFIG macro defined on the ImGui target.
//
// See ${ImGui_SOURCE_DIR}/imconfig.h for the full list of available options.
// Keep this file minimal: only override what we actually want to change.

#pragma once

// Strip obsolete API surface so we catch deprecated usage at compile time.
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

// NOTE: we intentionally do NOT define IMGUI_DISABLE_DEFAULT_FONT here.
// Germen disables it because it embeds IBM Plex Sans + Noto Color Emoji + BabelStone
// at build time. The PoC ships no embedded fonts yet, so we rely on ImGui's
// default ProggyClean to avoid an empty atlas at startup. Will revisit when we
// integrate font embedding.
