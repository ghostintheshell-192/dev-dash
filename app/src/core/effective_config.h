#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "config_layer.h"

namespace dev_dash::core
{
    struct ConfigNode
    {
        std::string relativePath;       // display label (file path, server name, hook event…)
        ConfigLayerKind sourceLayer;
        std::filesystem::path sourceFilePath;  // file to open on click
        bool shadowed = false;          // same name defined at a higher-precedence layer
        std::string note;               // short qualifier shown next to the label

        ConfigNode(std::string label,
                   ConfigLayerKind layer,
                   std::filesystem::path file)
            : relativePath(std::move(label))
            , sourceLayer(layer)
            , sourceFilePath(std::move(file))
        {
        }
    };

    enum class ConfigSectionKind
    {
        kClaudeMd,
        kRules,
        kMemory,
        kSkills,
        kAgents,
        kMcpServers,
        kHooks,
    };

    struct ConfigSection
    {
        ConfigSectionKind kind;
        std::string label;
        std::vector<ConfigNode> nodes;
        bool alwaysInContext;   // false → on-demand / zero tokens until triggered
    };

    class EffectiveConfig
    {
    public:
        EffectiveConfig() = default;
        explicit EffectiveConfig(std::vector<ConfigSection> sections)
            : _sections(std::move(sections)) {}

        const std::vector<ConfigSection>& Sections() const { return _sections; }

    private:
        std::vector<ConfigSection> _sections;
    };
}
