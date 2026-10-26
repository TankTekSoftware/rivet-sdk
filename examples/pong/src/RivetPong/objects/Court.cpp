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

    const float display_width = static_cast<float>(GetScreenWidth());
    const float display_height = static_cast<float>(GetScreenHeight());

    DrawLineV({ 0.0f, display_height / 2.0f }, { display_width, display_height / 2.0f }, WHITE);

    // The opponent defends the top half of the court, and the player the bottom half.
    this->render_score(this->opponent_score, display_height * 0.25f);
    this->render_score(this->player_score, display_height * 0.75f);
}

void Court::render_score(int score, float center_y)
{
    const int font_size = 120;
    const Color color = { 255, 255, 255, 50 };

    const char* text = TextFormat("%i", score);
    const int x = (GetScreenWidth() - MeasureText(text, font_size)) / 2;
    const int y = static_cast<int>(center_y) - font_size / 2;

    DrawText(text, x, y, font_size, color);
}
