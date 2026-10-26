#pragma once

#include <string_view>

namespace Rivet
{
    class System
    {
        public:
            static std::string_view get_os_name();
    };
}