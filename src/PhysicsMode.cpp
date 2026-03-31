#include "PhysicsMode.h"
#include <cstdlib>
#include <cmath>
#include <print>

PhysicsMode::PhysicsMode(int winW, int winH)
    : winW_(winW), winH_(winH), world_(b2Vec2(0.0f, 8.0f))
{
    const float wW = winW_ / PPM;
    const float wH = winH_ / PPM;

    makeWall(wW * 0.5f,           -WALL_M * 0.5f,      wW * 0.5f + WALL_M, WALL_M * 0.5f); // top
    makeWall(wW * 0.5f,            wH + WALL_M * 0.5f, wW * 0.5f + WALL_M, WALL_M * 0.5f); // bottom
    makeWall(-WALL_M * 0.5f,       wH * 0.5f,          WALL_M * 0.5f, wH * 0.5f + WALL_M); // left
    makeWall( wW + WALL_M * 0.5f,  wH * 0.5f,          WALL_M * 0.5f, wH * 0.5f + WALL_M); // right

    balls_.push_back({ makeBall(240.0f / PPM, 290.0f / PPM,  2.0f,  1.5f), { 220,  80,  60, 255 } });
    balls_.push_back({ makeBall(560.0f / PPM, 290.0f / PPM, -1.5f,  2.2f), {  60, 130, 200, 255 } });
}

void PhysicsMode::update(float dt)
{
    world_.Step(dt, 8, 3);
}

void PhysicsMode::render(SDL_Renderer* renderer)
{
    for (const Ball& ball : balls_) {
        b2Vec2 pos = ball.body->GetPosition();
        int cx = (int)(pos.x * PPM);
        int cy = (int)(pos.y * PPM);
        SDL_SetRenderDrawColor(renderer, ball.color.r, ball.color.g, ball.color.b, 255);
        //SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        fillCircle(renderer, cx, cy, BALL_PX / 2);
    }
}

bool PhysicsMode::handleCommand(const std::vector<std::string>& tokens)
{
    if (tokens[0] != "spawn_ball") return false;

    float cx = (tokens.size() > 1) ? std::stof(tokens[1]) : (float)(rand() % winW_);
    float cy = (tokens.size() > 2) ? std::stof(tokens[2]) : (float)(rand() % winH_);
    float vx = (tokens.size() > 3) ? std::stof(tokens[3]) : (float)((rand() % 400 - 200) / 100.0f);
    float vy = (tokens.size() > 4) ? std::stof(tokens[4]) : (float)((rand() % 400 - 200) / 100.0f);
    Uint8 r  = (tokens.size() > 5) ? (Uint8)std::stoi(tokens[5]) : (Uint8)(rand() % 256);
    Uint8 g  = (tokens.size() > 6) ? (Uint8)std::stoi(tokens[6]) : (Uint8)(rand() % 256);
    Uint8 b  = (tokens.size() > 7) ? (Uint8)std::stoi(tokens[7]) : (Uint8)(rand() % 256);

    balls_.push_back({ makeBall(cx / PPM, cy / PPM, vx, vy), { r, g, b, 255 } });
    return true;
}

b2Body* PhysicsMode::makeBall(float cx, float cy, float vx, float vy)
{
    b2BodyDef bd;
    bd.type           = b2_dynamicBody;
    bd.position.Set(cx, cy);
    bd.linearVelocity.Set(vx, vy);
    bd.linearDamping  = 0.0f;
    bd.angularDamping = 0.0f;
    b2Body* body = world_.CreateBody(&bd);

    b2CircleShape shape;
    shape.m_radius = BALL_R;

    b2FixtureDef fd;
    fd.shape       = &shape;
    fd.density     = 1.0f;
    fd.friction    = 0.0f;
    fd.restitution = 1.0f;
    body->CreateFixture(&fd);
    return body;
}

void PhysicsMode::makeWall(float cx, float cy, float hw, float hh)
{
    b2BodyDef bd;
    bd.position.Set(cx, cy);
    b2Body* body = world_.CreateBody(&bd);

    b2PolygonShape shape;
    shape.SetAsBox(hw, hh);

    b2FixtureDef fd;
    fd.shape       = &shape;
    fd.friction    = 0.0f;
    fd.restitution = 1.0f;
    body->CreateFixture(&fd);
}

void PhysicsMode::fillCircle(SDL_Renderer* renderer, int cx, int cy, int r)
{
    for (int dy = -r; dy <= r; dy++) {
        int dx = (int)std::sqrt((double)(r * r - dy * dy));
        SDL_RenderDrawLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}
