#pragma once

#include "bbcode.h"
#include "color_scheme.h"
#include "mode.h"
#include "renderer.h"
#include <string>
#include <vector>

class ListControl : public Mode {
public:
    explicit ListControl(const ColorScheme& scheme = scheme::night) : scheme_(scheme) {}

    bool on_command(std::string_view command, std::string_view args) override;
    void update(float dt) override;
    void render(Renderer& r) override;

private:
    ColorScheme scheme_;

    enum class State { Hidden, Visible };

    struct ListItem {
        std::wstring text;
        std::vector<TextSpan> spans;
    };

    static std::wstring to_wide(std::string_view s);

    State state_ = State::Hidden;
    std::wstring title_;
    std::vector<ListItem> items_;
    float x_ = 0.f;
    float y_ = 0.f;
    float width_ = 400.f;

    static constexpr float kPad = 16.f;
    static constexpr float kTitlePad = 6.f;
    static constexpr float kItemHeight = 32.f;
    static constexpr float kItemGap = 4.f;
    static constexpr float kCornerRadius = 0.f;
    static constexpr float kTitleHeight = 39.f;

    float shadow_blur_ = 7.f;
    float shadow_offset_x_ = 4.f;
    float shadow_offset_y_ = 4.f;
    D2D1_COLOR_F shadow_color_ = {0.f, 0.f, 0.f, 1.f};
};
