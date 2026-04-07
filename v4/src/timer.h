#pragma once

#include "mode.h"
#include "renderer.h"
#include <string>

class Timer : public Mode {
public:
    bool on_command(std::string_view command, std::string_view args) override;
    void update(float dt) override;
    void render(Renderer& r) override;

private:
    enum class State { Hidden, Visible };

    static std::wstring to_wide(std::string_view s);

    State state_ = State::Hidden;
    std::wstring label_;
    float remaining_ = 0.f;
    float x_ = 0.f;
    float y_ = 0.f;
};
