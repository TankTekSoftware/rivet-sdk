#pragma once

#include <Rivet.hpp>

class Court
{
    private:
        int player_score;
        int opponent_score;
    public:
        Court();
    public:
        void add_player_point();
        void add_opponent_point();
    public:
        void render(Rivet::RenderPass pass);
    private:
        void render_score(int score, float center_y);
};