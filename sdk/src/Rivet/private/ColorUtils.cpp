#include <Rivet/private/ColorUtils.hpp>

#include <algorithm>

namespace Rivet
{
    unsigned char ColorUtils::to_byte(float value)
    {
        float clamped = std::clamp(value, 0.0f, 1.0f);
        return static_cast<unsigned char>(clamped * 255.0f + 0.5f);
    }

    ::Color ColorUtils::to_rlcolor(const Color& color)
    {
        unsigned char r = to_byte(color.r);
        unsigned char g = to_byte(color.g);
        unsigned char b = to_byte(color.b);
        unsigned char a = to_byte(color.a);

        return (::Color) { r, g, b, a };
    }
}