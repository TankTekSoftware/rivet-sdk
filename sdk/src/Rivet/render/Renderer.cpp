#include <Rivet/render/Renderer.hpp>

#include <Rivet/private/ColorUtils.hpp>

#include <raylib.h>

namespace Rivet
{
    Renderer& Renderer::get_singleton()
    {
        static Renderer singleton;
        return singleton;
    }

    Renderer::Renderer()
    {
        this->clear_color = Color();
    }

    void Renderer::begin()
    {
        BeginDrawing();
        Renderer::clear();
    }

    void Renderer::end()
    {
        EndDrawing();
    }

    void Renderer::set_default_clear_color(float r, float g, float b, float a)
    {
        Renderer& self = Renderer::get_singleton();
        self.clear_color.r = r;
        self.clear_color.g = g;
        self.clear_color.b = b;
        self.clear_color.a = a;
    }

    void Renderer::set_default_clear_color(const Color& color)
    {
        Renderer& self = Renderer::get_singleton();
        self.clear_color = color;
    }

    void Renderer::clear()
    {
        Renderer& self = Renderer::get_singleton();
        Renderer::clear(self.clear_color);
    }

    void Renderer::clear(float r, float g, float b, float a)
    {
        Renderer::clear(Color(r, g, b, a));
    }

    void Renderer::clear(const Color& color)
    {
        ClearBackground(ColorUtils::to_rlcolor(color));
    }

    void Renderer::debug_fps(int x, int y)
    {
        DrawFPS(x, y);
    }

}