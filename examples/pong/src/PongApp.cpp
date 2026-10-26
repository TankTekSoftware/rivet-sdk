#include <Rivet.hpp>
#include <Rivet/EntryPoint.hpp>

class PongApp : public Rivet::Application
{
    protected:
        void on_compose() override
        {
            Rivet::Display::set_title("Rivet Pong");
            Rivet::Display::set_size(360, 640);
            Rivet::Display::set_vsync(true);

            Rivet::Viewport::set_size(720, 1280);
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
            if (pass == Rivet::RenderPass::SCREEN) {
                ::DrawFPS(24, 24);
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
