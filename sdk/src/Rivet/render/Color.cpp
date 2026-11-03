#include <Rivet/render/Color.hpp>

namespace Rivet
{
    Color::Color()
    {
        this->r = 0.392f;
        this->g = 0.584f;
        this->b = 0.929f;
        this->a = 1.0f;
    }

    Color::Color(float r, float g, float b, float a)
    {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }
}