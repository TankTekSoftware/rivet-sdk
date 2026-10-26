#include <Rivet/utils/Time.hpp>

#include <raylib.h>

namespace Rivet
{
    float Time::get_delta()
    {
        return GetFrameTime();
    }
}