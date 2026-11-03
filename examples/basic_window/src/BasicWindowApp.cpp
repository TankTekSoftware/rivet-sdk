#include <Rivet.hpp>
#include <Rivet/EntryPoint.hpp>

class PongApp : public Rivet::Application
{
    protected:
        void on_compose() override
        {
            // This block of code configures how the window should be setup.
            Rivet::Display::set_title("Rivet - Basic Window");
            Rivet::Display::set_size(1280, 720);
            Rivet::Display::set_fullscreen(false);
            Rivet::Display::set_resizable(false);
            Rivet::Display::set_vsync(true);

            // This block of code configures the viewport. This is where things are rendered to.
            Rivet::Viewport::set_size(1280, 720);
            Rivet::Viewport::set_integer_scaling(false);
            Rivet::Viewport::set_pixel_filter(false);
        }

        void on_ready() override
        {
            
        }

        void on_update(float delta) override
        {
            
        }

        void on_render(Rivet::RenderPass pass) override
        {
            if (pass == Rivet::RenderPass::VIEWPORT) {
                Rivet::Renderer::debug_fps(2, 2);
            }
        }

        void on_dispose() override
        {
           
        }
};

Rivet::Application* Rivet::create_application()
{
    return new PongApp();
}
