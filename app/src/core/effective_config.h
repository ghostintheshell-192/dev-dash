#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "config_layer.h"

namespace dev_dash::core
{
    struct ConfigNode
    {
        std::string relativePath;
        ConfigLayerKind sourceLayer;
        std::filesystem::path sourceFilePath;
    };

    class EffectiveConfig
    {
    public:
        EffectiveConfig() = default;

        explicit EffectiveConfig(std::vector<ConfigNode> nodes)
            : _nodes(std::move(nodes))
        {
            for (std::size_t i = 0; i < _nodes.size(); ++i)
            {
                for (std::size_t j = i + 1; j < _nodes.size(); ++j)
                {
                    if (_nodes[i].relativePath == _nodes[j].relativePath)
                        throw std::invalid_argument("Duplicate relativePath: " + _nodes[i].relativePath);
                }
            }
        }

        const std::vector<ConfigNode>& Nodes() const { return _nodes; }

        const ConfigNode* FindNode(std::string_view relativePath) const
        {
            for (const auto& n : _nodes)
                if (n.relativePath == relativePath)
                    return &n;
            return nullptr;
        }

        std::vector<const ConfigNode*> NodesFromLayer(ConfigLayerKind layer) const
        {
            std::vector<const ConfigNode*> result;
            for (const auto& n : _nodes)
                if (n.sourceLayer == layer)
                    result.push_back(&n);
            return result;
        }

        std::optional<ConfigLayerKind> SourceLayerOf(std::string_view relativePath) const
        {
            const ConfigNode* n = FindNode(relativePath);
            if (!n)
                return std::nullopt;
            return n->sourceLayer;
        }

    private:
        std::vector<ConfigNode> _nodes;
    };
}
