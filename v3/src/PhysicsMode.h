#pragma once
#include "Mode.h"
#include <box2d/box2d.h>
#include <vector>

struct Ball {
    b2Body*   body;
    SDL_Color color;
};

class PhysicsMode : public Mode {
public:
    PhysicsMode(int winW, int winH);

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;
    bool handleCommand(const std::vector<std::string>& tokens) override;

private:
    static constexpr float PPM     = 100.0f;
    static constexpr int   BALL_PX = 180;
    static constexpr float BALL_R  = (BALL_PX / PPM) * 0.5f;
    static constexpr float WALL_M  = 1.0f;

    int     winW_, winH_;
    b2World world_;

    std::vector<Ball> balls_;

    b2Body* makeBall(float cx, float cy, float vx, float vy);
    void    makeWall(float cx, float cy, float hw, float hh);
    void    fillCircle(SDL_Renderer* renderer, int cx, int cy, int r);
};
