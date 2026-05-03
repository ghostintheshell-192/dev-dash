// Adapted from Germen Pulchrum (DPD85/Germen @ 037827b, MIT) — `CodaCancellazione`.
// See poc/THIRD_PARTY_NOTICES.md for attribution details.

#pragma once

#include <functional>
#include <stack>
#include <utility>

class DeletionQueue
{
  public:
    using Deleter = std::function<void()>;

    DeletionQueue() = default;
    ~DeletionQueue() { Flush(); }

    DeletionQueue(const DeletionQueue &)            = delete;
    DeletionQueue &operator=(const DeletionQueue &) = delete;

    void Add(Deleter deleter) { _stack.push(std::move(deleter)); }

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
