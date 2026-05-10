#pragma once

// Adapted from Germen Pulchrum (DPD85/Germen, MIT) — `CodaCancellazione`.
// See app/THIRD_PARTY_NOTICES.md for attribution.

#include <functional>
#include <stack>

namespace dev_dash::platform
{
    class DeletionQueue
    {
    public:
        using Deleter = std::function<void()>;

        DeletionQueue() = default;
        ~DeletionQueue() { Flush(); }

        DeletionQueue(const DeletionQueue&)            = delete;
        DeletionQueue& operator=(const DeletionQueue&) = delete;

        void Add(Deleter d) { _stack.push(std::move(d)); }

        void Flush()
        {
            while (!_stack.empty())
            {
                _stack.top()();
                _stack.pop();
            }
        }

    private:
        std::stack<Deleter> _stack;
    };
}
