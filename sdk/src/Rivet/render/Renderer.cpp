#include <Rivet/render/Renderer.hpp>

#include <raylib.h>

namespace Rivet
{
    void Renderer::begin()
    {
        BeginDrawing();
        ClearBackground(BLACK);
    }

    void Renderer::end()
    {
        EndDrawing();
    }

}