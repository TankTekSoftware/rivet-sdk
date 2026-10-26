#pragma once

#include <Rivet.hpp>

class Ball
{
    private:
        Vector2 position;
        Vector2 velocity;
        float speed_multiplier;
        float radius;
    public:
        Ball();
    public:
        Vector2 get_position() const;
        Vector2 get_velocity() const;
    public:
        void update(float delta);
        void render(Rivet::RenderPass pass);
        void bounce_off(Rectangle rect);
        void reset();
};