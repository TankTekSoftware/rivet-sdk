#include <RivetPong/objects/AIPaddle.hpp>

// The AI is slower than the ball moves sideways, so a well-angled shot can get past it.
AIPaddle::AIPaddle(const Ball* ball)
    : Paddle(128.0f, 512.0f)
{
    this->ball = ball;
}

void AIPaddle::update(float delta)
{
    const Vector2 ball_position = this->ball->get_position();
    const Vector2 ball_velocity = this->ball->get_velocity();

    // Chase the ball while it's heading toward this paddle, otherwise drift back to the middle
    //      and wait for the return.
    const bool ball_incoming = (this->position.y - ball_position.y) * ball_velocity.y > 0.0f;
    const float target_x = ball_incoming ? ball_position.x : GetScreenWidth() / 2.0f;

    this->move_toward(target_x, delta);
}
