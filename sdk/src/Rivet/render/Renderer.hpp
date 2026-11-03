#pragma once

#include <Rivet/render/Color.hpp>

namespace Rivet
{
    class Renderer
    {
        private:
            Color clear_color;
        private:
            static Renderer& get_singleton();
        private:
            Renderer();
        public:
            static void begin();
            static void end();
        public:
            static void set_default_clear_color(float r, float g, float b, float a = 1.0f);
            static void set_default_clear_color(const Color& color);
            static void clear();
            static void clear(float r, float g, float b, float a = 1.0f);
            static void clear(const Color& color);
        public:
            static void debug_fps(int x, int y);
    };
}