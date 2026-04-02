#pragma once

#include "mode.h"
#include "renderer.h"
#include <deque>
#include <string>

class BottomPopup : public Mode {
public:
    bool on_command(std::string_view command, std::string_view args) override;
    void update(float dt) override;
    void render(Renderer& r) override;

private:
    enum class State { Idle, SlideIn, Display, SlideOut };

    struct Popup {
        std::wstring title;
        std::wstring body;
        D2D1_COLOR_F border_color = colors::accent;
        D2D1_COLOR_F bg_color     = colors::bg;
    };

    static D2D1_COLOR_F parse_color(std::string_view s, D2D1_COLOR_F fallback);
    static std::wstring to_wide(std::string_view s);

    State state_ = State::Idle;
    Popup current_;
    std::deque<Popup> pending_;

    float anim_t_   = 0.f;   // 0..1 animation progress
    float display_t_ = 0.f;  // seconds displayed

    static constexpr float kSlideTime   = 0.35f;  // seconds
    static constexpr float kDisplayTime = 5.0f;    // seconds
    static constexpr float kPopupWidth  = 420.f;
    static constexpr float kPopupHeight = 100.f;
    static constexpr float kMargin      = 0.f;
};
