#include "settings_parser.h"

#include <fstream>

#include <nlohmann/json.hpp>

namespace dev_dash::services
{
    namespace
    {
        nlohmann::json TryLoad(const std::filesystem::path& path)
        {
            if (!std::filesystem::exists(path)) return {};
            std::ifstream f(path);
            if (!f) return {};
            try { return nlohmann::json::parse(f); }
            catch (...) { return {}; }
        }
    }

    std::vector<core::ConfigNode> SettingsParser::ParseMcpServers(
        const std::filesystem::path& settingsPath,
        core::ConfigLayerKind layer) const
    {
        std::vector<core::ConfigNode> result;
        const auto json = TryLoad(settingsPath);
        if (json.is_null() || !json.contains("mcpServers")) return result;

        for (const auto& [name, _] : json["mcpServers"].items())
            result.push_back({name, layer, settingsPath});

        return result;
    }

    std::vector<core::ConfigNode> SettingsParser::ParseHooks(
        const std::filesystem::path& settingsPath,
        core::ConfigLayerKind layer) const
    {
        std::vector<core::ConfigNode> result;
        const auto json = TryLoad(settingsPath);
        if (json.is_null() || !json.contains("hooks")) return result;

        for (const auto& [event, groups] : json["hooks"].items())
        {
            if (!groups.is_array()) continue;
            for (const auto& group : groups)
            {
                std::string label = event;
                if (group.contains("matcher") && group["matcher"].is_string())
                    label += " / " + group["matcher"].get<std::string>();
                result.push_back({label, layer, settingsPath});
            }
        }

        return result;
    }
}
