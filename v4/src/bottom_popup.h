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
        //D2D1_COLOR_F border_color = colors::accent;
        D2D1_COLOR_F border_color = colors::bg;
        D2D1_COLOR_F bg_color     = colors::bg;
    };

    static D2D1_COLOR_F parse_color(std::string_view s, D2D1_COLOR_F fallback);
    static std::wstring to_wide(std::string_view s);

    State state_ = State::Idle;
    Popup current_;
    std::deque<Popup> pending_;

    float anim_t_   = 0.f;   // 0..1 animation progress
    float display_t_ = 0.f;  // seconds displayed

    static constexpr float kSlideTime    = 0.35f;  // seconds
    static constexpr float kDisplayTime  = 5.0f;   // seconds
    static constexpr float kPad          = 30.f;
    static constexpr float kTitleHeight  = 28.f;
    static constexpr float kTitleBodyGap = 4.f;
    static constexpr float kBodyHeight   = 40.f;
    static constexpr float kPopupWidth   = 1920.f;
    static constexpr float kPopupHeight  = kPad + kTitleHeight + kTitleBodyGap + kBodyHeight + kPad;
    static constexpr float kMargin       = 0.f;
};
