#pragma once

#include <Rivet/render/Color.hpp>

namespace Rivet
{
    class Renderer
    {
        public:
            static void begin();
            static void end();
        public:
            static void clear();
            static void clear(float r, float g, float b, float a = 1.0f);
            static void clear(const Color& color);
        public:
            static void debug_fps(int x, int y);
    };
}