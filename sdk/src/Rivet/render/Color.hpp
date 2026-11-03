#pragma once

namespace Rivet
{
    class Color
    {
        public:
            float r;
            float g;
            float b;
            float a;
        public:
            Color(float r, float g, float b, float a = 1.0f);
    };
}