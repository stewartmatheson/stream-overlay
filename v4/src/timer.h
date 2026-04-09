#pragma once

#include "color_scheme.h"
#include "mode.h"
#include "renderer.h"
#include <string>

class Timer : public Mode {
public:
    explicit Timer(const ColorScheme& scheme = scheme::night) : scheme_(scheme) {}

    bool on_command(std::string_view command, std::string_view args) override;
    void update(float dt) override;
    void render(Renderer& r) override;

private:
    enum class State { Hidden, Visible };

    static std::wstring to_wide(std::string_view s);

    ColorScheme scheme_;
    State state_ = State::Hidden;
    std::wstring label_;
    float remaining_ = 0.f;
    float x_ = 0.f;
    float y_ = 0.f;
};
