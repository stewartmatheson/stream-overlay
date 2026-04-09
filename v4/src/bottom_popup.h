#pragma once

#include "bbcode.h"
#include "color_scheme.h"
#include "mode.h"
#include "renderer.h"
#include <deque>
#include <string>

class BottomPopup : public Mode {
public:
    explicit BottomPopup(const ColorScheme& scheme = scheme::day) : scheme_(scheme) {}

    bool on_command(std::string_view command, std::string_view args) override;
    void update(float dt) override;
    void render(Renderer& r) override;

private:
    ColorScheme scheme_;

    enum class State { Idle, SlideIn, Display, SlideOut };

    struct Popup {
        std::wstring title;
        std::wstring body;
        std::vector<TextSpan> body_spans;
        D2D1_COLOR_F bg_color     = {};
        float top_border_thickness = 5.f;
        D2D1_COLOR_F top_border_color = {};
        float display_time        = 0.f;
    };

    static D2D1_COLOR_F parse_color(std::string_view s, D2D1_COLOR_F fallback);
    static D2D1_COLOR_F parse_hex_color(std::string_view s, D2D1_COLOR_F fallback);
    static std::wstring to_wide(std::string_view s);
    static int count_words(std::string_view s);
    static float compute_display_time(std::string_view title, std::string_view body);

    State state_ = State::Idle;
    Popup current_;
    std::deque<Popup> pending_;

    float anim_t_   = 0.f;   // 0..1 animation progress
    float display_t_ = 0.f;  // seconds displayed

    static constexpr float kSlideTime      = 0.35f;  // seconds
    static constexpr float kReadingWPM     = 150.f;  // words per minute
    static constexpr float kMinDisplayTime = 3.0f;   // seconds floor
    static constexpr float kDisplayPadding = 1.5f;   // extra seconds for noticing the popup
    static constexpr float kPad          = 30.f;
    static constexpr float kTitleHeight  = 28.f;
    static constexpr float kTitleBodyGap = 4.f;
    static constexpr float kBodyHeight   = 40.f;
    static constexpr float kPopupWidth   = 1920.f;
    static constexpr float kPopupHeight  = kPad + kTitleHeight + kTitleBodyGap + kBodyHeight + kPad;
    static constexpr float kMargin       = 0.f;
};
