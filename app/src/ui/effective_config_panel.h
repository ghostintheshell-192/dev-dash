#pragma once

#include "../core/config_layer.h"
#include "../core/effective_config.h"
#include "../core/project.h"

namespace dev_dash::services { class ConfigResolver; }

namespace dev_dash::ui
{
    class DocumentPanelHost;

    class EffectiveConfigPanel
    {
    public:
        EffectiveConfigPanel(services::ConfigResolver& resolver,
                             DocumentPanelHost& docHost,
                             const core::Project& project);

        void Render();
        bool WantsBack()         const { return _wantsBack; }
        bool WantsScaffoldDiff() const { return _wantsScaffoldDiff; }
        const core::Project& Project() const { return _project; }

    private:
        void Refresh();

        services::ConfigResolver& _resolver;
        DocumentPanelHost&        _docHost;
        core::Project             _project;
        core::EffectiveConfig     _config;
        bool                      _wantsBack         = false;
        bool                      _wantsScaffoldDiff = false;
    };
}
