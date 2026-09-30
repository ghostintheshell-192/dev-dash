#include "hooks_adapter.h"

namespace dev_dash::services
{
    HooksAdapter::HooksAdapter(SettingsParser& parser)
        : _parser(parser)
    {}

    core::ConfigSection HooksAdapter::Resolve(
        const core::ConfigLayer& global,
        const core::ConfigLayer& project) const
    {
        core::ConfigSection section{
            core::ConfigSectionKind::kHooks, "Hooks", {}, false
        };

        auto addLayer = [&](const core::ConfigLayer& layer)
        {
            if (layer.path.empty()) return;
            for (const auto& fname : {"settings.json", "settings.local.json"})
            {
                for (auto& node : _parser.ParseHooks(layer.path / fname, layer.kind))
                    section.nodes.push_back(std::move(node));
            }
        };

        addLayer(global);
        addLayer(project);

        return section;
    }
}
