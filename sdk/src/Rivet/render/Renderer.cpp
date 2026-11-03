#include <Rivet/render/Renderer.hpp>

#include <Rivet/private/ColorUtils.hpp>

#include <raylib.h>

namespace Rivet
{
    void Renderer::begin()
    {
        BeginDrawing();
        Renderer::clear();
    }

    void Renderer::end()
    {
        EndDrawing();
    }

    void Renderer::clear()
    {
        Renderer::clear(Color(0.392f, 0.584f, 0.929f));
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