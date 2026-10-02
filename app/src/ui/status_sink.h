#pragma once

#include <string>
#include <utility>

namespace dev_dash::ui
{
    // App-wide status line, rendered by the shell's status bar. Views report
    // operation outcomes here instead of each owning a transient inline
    // message — errors stay visible until something else happens.
    //
    // The bar is also where the app explains itself, instead of tooltips: a
    // hint (what the item under the mouse does, an activity running in the
    // background) takes the place of the message while it is set. Hints are
    // set again every frame they apply, and the bar takes them when it draws,
    // so a hint goes away by itself when the mouse leaves its item.
    class StatusSink
    {
    public:
        enum class Level { kInfo, kSuccess, kError };

        void Set(Level level, std::string message)
        {
            _level   = level;
            _message = std::move(message);
        }

        void Clear() { _message.clear(); }

        // The last hint set wins: an item under the mouse overrides an
        // activity set earlier in the frame.
        void SetHint(std::string hint) { _hint = std::move(hint); }
        std::string TakeHint() { return std::exchange(_hint, std::string()); }

        Level              GetLevel() const { return _level; }
        const std::string& Message() const { return _message; }

    private:
        Level       _level = Level::kInfo;
        std::string _message;
        std::string _hint;
    };
}
