#pragma once

#include <Rivet/render/Color.hpp>

#include <raylib.h>

namespace Rivet
{
    class ColorUtils
    {
        public:
            static unsigned char to_byte(float value);
        public:
            static ::Color to_rlcolor(const Color& color);
    };
}