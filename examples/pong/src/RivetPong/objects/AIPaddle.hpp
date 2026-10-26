#pragma once

#include <RivetPong/objects/Paddle.hpp>
#include <RivetPong/objects/Ball.hpp>

class AIPaddle : public Paddle
{
    private:
        const Ball* ball;
    public:
        AIPaddle(const Ball* ball);
    public:
        void update(float delta) override;
};