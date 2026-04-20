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
    std::vector<ListItem> items_;
    float x_ = 0.f;
    float y_ = 0.f;
    float width_ = 400.f;

    static constexpr float kPad = 16.f;
    static constexpr float kItemHeight = 32.f;
    static constexpr float kItemGap = 4.f;
    static constexpr float kCornerRadius = 8.f;
};
