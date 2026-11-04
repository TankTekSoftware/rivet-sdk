#pragma once

#include <sol/sol.hpp>

namespace RivetRuntime
{
    // The `rivet` table that scripts see. Every function here forwards to the SDK, so the shape of
    //      the C++ API is the shape of the Lua API; where the SDK has no wrapper yet the binding
    //      calls raylib directly, and those are the calls to move into the SDK first.
    class Bindings
    {
        public:
            static void install(sol::state_view lua);
    };
}
