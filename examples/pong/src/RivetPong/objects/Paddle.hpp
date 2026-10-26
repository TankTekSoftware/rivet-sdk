#pragma once

#include <Rivet.hpp>

class Paddle
{
    protected:
        Vector2 position;
        Vector2 size;
        float speed;
    public:
        Paddle(float y, float speed);
        virtual ~Paddle() = default;
    public:
        Rectangle get_rect() const;
    public:
        virtual void update(float delta) = 0;
        void render(Rivet::RenderPass pass);
    protected:
        void move_toward(float target_x, float delta);
};