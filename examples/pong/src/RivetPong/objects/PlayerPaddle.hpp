#pragma once

#include <RivetPong/objects/Paddle.hpp>

class PlayerPaddle : public Paddle
{
    public:
        PlayerPaddle();
    public:
        void update(float delta) override;
};