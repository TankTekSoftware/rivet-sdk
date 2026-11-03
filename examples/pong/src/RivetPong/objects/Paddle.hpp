#pragma once

#include <Rivet.hpp>

#include <raylib.h>

class Paddle
{
    protected:
        Vector2 position;
        Vector2 size;
        float speed;
        float center_offset;
    public:
        Paddle(float center_offset, float speed);
        virtual ~Paddle() = default;
    public:
        Rectangle get_rect() const;
    public:
        virtual void update(float delta) = 0;
        void render(Rivet::RenderPass pass);
    protected:
        void follow_center();
        void move_toward(float target_x, float delta);
};