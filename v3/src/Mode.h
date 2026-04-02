#pragma once
#include <SDL.h>
#include <vector>
#include <string>

class Mode {
public:
    virtual ~Mode() = default;
    virtual void update(float dt) = 0;
    virtual void render(SDL_Renderer* renderer) = 0;
    virtual bool handleCommand(const std::vector<std::string>& tokens) = 0;
    virtual bool isIdle() const { return true; }
};
