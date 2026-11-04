#include <RivetPong/objects/Court.hpp>

Court::Court()
{
    this->player_score = 0;
    this->opponent_score = 0;
}

void Court::add_player_point()
{
    this->player_score++;
}

void Court::add_opponent_point()
{
    this->opponent_score++;
}

void Court::render(Rivet::RenderPass pass)
{
    if (pass != Rivet::RenderPass::VIEWPORT) {
        return;
    }

    const float viewport_width = static_cast<float>(Rivet::Viewport::get_width());
    const float viewport_height = static_cast<float>(Rivet::Viewport::get_height());

    const float line_thickness = 2.0f;
    DrawLineEx({ 0.0f, viewport_height / 2.0f }, { viewport_width, viewport_height / 2.0f }, line_thickness, WHITE);

    // The opponent defends the top half of the court, and the player the bottom half.
    this->render_score(this->opponent_score, viewport_height * 0.25f);
    this->render_score(this->player_score, viewport_height * 0.75f);
}

void Court::render_score(int score, float center_y)
{
    const int font_size = 240;
    const Color color = { 255, 255, 255, 50 };

    const char* text = TextFormat("%i", score);
    const int viewport_width = static_cast<int>(Rivet::Viewport::get_width());
    const int x = (viewport_width - MeasureText(text, font_size)) / 2;
    const int y = static_cast<int>(center_y) - font_size / 2;

    DrawText(text, x, y, font_size, color);
}
