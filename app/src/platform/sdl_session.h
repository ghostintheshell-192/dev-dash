#pragma once

namespace dev_dash::platform
{
    class SdlSession
    {
    public:
        SdlSession();
        ~SdlSession();

        SdlSession(const SdlSession&)            = delete;
        SdlSession& operator=(const SdlSession&) = delete;
    };
}
