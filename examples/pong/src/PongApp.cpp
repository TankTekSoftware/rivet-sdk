#include <Rivet.hpp>
#include <Rivet/EntryPoint.hpp>

#include <RivetPong/objects/Ball.hpp>
#include <RivetPong/objects/Court.hpp>
#include <RivetPong/objects/PlayerPaddle.hpp>
#include <RivetPong/objects/AIPaddle.hpp>

class PongApp : public Rivet::Application
{
    private:
        Court* court = nullptr;
        Ball* ball = nullptr;
        PlayerPaddle* player = nullptr;
        AIPaddle* opponent = nullptr;
    protected:
        void on_compose() override
        {
            Rivet::Renderer::set_default_clear_color(0.1f, 0.1f, 0.1f);

            Rivet::Display::set_title("Rivet Pong");
            Rivet::Display::set_size(360, 640);
            Rivet::Display::set_resizable(true);
            Rivet::Display::set_vsync(true);

            // The court is always 720 wide, and grows taller so phones taller than 9:16 don't get
            //      bars. Screens wider than 9:16 get bars on the sides instead, since a wider court
            //      would move the walls the ball bounces off.
            Rivet::Viewport::set_size(720, 1280);
            Rivet::Viewport::set_stretch_mode(Rivet::StretchMode::CANVAS_ITEMS);
            Rivet::Viewport::set_stretch_aspect(Rivet::StretchAspect::KEEP_WIDTH);
            Rivet::Viewport::set_stretch_scale(1.0f);
            Rivet::Viewport::set_integer_scaling(false);
            Rivet::Viewport::set_pixel_filter(false);
        }

        void on_ready() override
        {
            this->court = new Court();
            this->ball = new Ball();
            this->player = new PlayerPaddle();
            this->opponent = new AIPaddle(this->ball);
        }

        void on_update(float delta) override
        {
            this->player->update(delta);
            this->opponent->update(delta);
            this->ball->update(delta);

            this->ball->bounce_off(this->player->get_rect());
            this->ball->bounce_off(this->opponent->get_rect());

            // A ball that gets past a paddle is a point for the other side.
            const float ball_y = this->ball->get_position().y;
            if (ball_y < 0.0f) {
                this->court->add_player_point();
                this->ball->reset();
            }
            else if (ball_y > static_cast<float>(Rivet::Viewport::get_height())) {
                this->court->add_opponent_point();
                this->ball->reset();
            }
        }

        void on_render(Rivet::RenderPass pass) override
        {
            if (pass == Rivet::RenderPass::VIEWPORT) {
                Rivet::Renderer::clear(0.0f, 0.0f, 0.0f);
                this->court->render(pass);
                this->ball->render(pass);
                this->player->render(pass);
                this->opponent->render(pass);
            }
        }

        void on_dispose() override
        {
            // The AI paddle holds a pointer to the ball, so it goes first.
            delete this->opponent;
            this->opponent = nullptr;

            delete this->player;
            this->player = nullptr;

            delete this->ball;
            this->ball = nullptr;

            delete this->court;
            this->court = nullptr;
        }
};

Rivet::Application* Rivet::create_application()
{
    return new PongApp();
}
