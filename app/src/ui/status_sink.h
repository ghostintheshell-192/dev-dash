#pragma once

#include <string>

namespace dev_dash::ui
{
    // App-wide status line, rendered by the shell's status bar. Views report
    // operation outcomes here instead of each owning a transient inline
    // message — errors stay visible until something else happens.
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

        Level              GetLevel() const { return _level; }
        const std::string& Message() const { return _message; }

    private:
        Level       _level = Level::kInfo;
        std::string _message;
    };
}
