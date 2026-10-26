#include <Rivet/core/Application.hpp>
#include <Rivet/core/Display.hpp>
#include <Rivet/core/Viewport.hpp>

#include <Rivet/render/Renderer.hpp>

#include <Rivet/utils/System.hpp>
#include <Rivet/utils/Time.hpp>

#include <rlImGui.h>
#include <imgui.h>
#include <raylib.h>

namespace Rivet
{
    void Application::create()
    {
        this->on_compose();
        Display::compose();
        Viewport::compose();
        rlImGuiSetup(true);

        this->on_ready();
    }

    void Application::tick(bool view_size_changed)
    {
        // UNUSED PARAMETERS: This parameter is required by some platforms (eg. iOS), but
        //      is unused by Rivet.
        (void)view_size_changed;   

        this->update();
        this->render();
    }

    void Application::destroy()
    {
        this->on_dispose();
        rlImGuiShutdown();
        Viewport::dispose();
        Display::dispose();
    }

    void Application::update()
    {
        // Get the time between the current frame and the last frame
        // at the start of the current frame.
        const float delta = Time::get_delta();

        // Apply a global update used across the entire game.
        this->on_update(delta);
        // TODO: Update current scene here.
    }

    void Application::render()
    {
        Viewport::begin();
        
        // RENDER PASS: This render pass is used for game elements or for anything that needs
        //      to be scaled with the game.
        this->on_render(RenderPass::VIEWPORT);
        
        Viewport::end();

        // --- SCREEN DRAWING --- //
        Renderer::begin();
        
        Viewport::render();

        // RENDER PASS: This render pass is used for anything
        //      that should be drawn directly to the screen.
        this->on_render(RenderPass::SCREEN);

        // RENDER PASS: This render pass is primary used for debugging.
        //      This is where the ImGui windows are drawn.
        rlImGuiBegin();
            this->on_render(RenderPass::IMGUI);
        rlImGuiEnd();

        Renderer::end();
    }

    void Application::run()
    {
        this->create();

        while (!Display::should_close()) {
            this->tick(false);
        }

        this->on_dispose();
    }
}