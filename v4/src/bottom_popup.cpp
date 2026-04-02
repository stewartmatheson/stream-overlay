#include "bottom_popup.h"
#include <algorithm>
#include <charconv>
#include <vector>

// Ease-out cubic
static float ease_out(float t) {
    float f = 1.f - t;
    return 1.f - f * f * f;
}

std::wstring BottomPopup::to_wide(std::string_view s) {
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring out(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), len);
    return out;
}

D2D1_COLOR_F BottomPopup::parse_color(std::string_view s, D2D1_COLOR_F fallback) {
    // Expected: "R,G,B" with 0-255 values
    float rgb[3];
    int idx = 0;
    size_t start = 0;
    for (size_t i = 0; i <= s.size() && idx < 3; ++i) {
        if (i == s.size() || s[i] == ',') {
            auto part = s.substr(start, i - start);
            int val = 0;
            auto [ptr, ec] = std::from_chars(part.data(), part.data() + part.size(), val);
            if (ec != std::errc()) return fallback;
            rgb[idx++] = std::clamp(val, 0, 255) / 255.f;
            start = i + 1;
        }
    }
    if (idx != 3) return fallback;
    return {rgb[0], rgb[1], rgb[2], fallback.a};
}

bool BottomPopup::on_command(std::string_view command, std::string_view args) {
    if (command != "bottompopup") return false;

    // Parse pipe-delimited fields: Title|Body[|BorderColor][|BgColor]
    std::vector<std::string_view> fields;
    size_t start = 0;
    for (size_t i = 0; i <= args.size(); ++i) {
        if (i == args.size() || args[i] == '|') {
            fields.push_back(args.substr(start, i - start));
            start = i + 1;
        }
    }

    if (fields.size() < 2) return false;

    Popup p;
    p.title = to_wide(fields[0]);
    p.body  = to_wide(fields[1]);
    if (fields.size() >= 3) p.border_color = parse_color(fields[2], colors::accent);
    if (fields.size() >= 4) p.bg_color     = parse_color(fields[3], colors::bg);

    if (state_ == State::Idle) {
        current_ = std::move(p);
        state_ = State::SlideIn;
        anim_t_ = 0.f;
    } else {
        pending_.push_back(std::move(p));
    }
    return true;
}

void BottomPopup::update(float dt) {
    switch (state_) {
    case State::Idle:
        if (!pending_.empty()) {
            current_ = std::move(pending_.front());
            pending_.pop_front();
            state_ = State::SlideIn;
            anim_t_ = 0.f;
        }
        break;

    case State::SlideIn:
        anim_t_ += dt / kSlideTime;
        if (anim_t_ >= 1.f) {
            anim_t_ = 1.f;
            state_ = State::Display;
            display_t_ = 0.f;
        }
        break;

    case State::Display:
        display_t_ += dt;
        if (display_t_ >= kDisplayTime) {
            state_ = State::SlideOut;
            anim_t_ = 0.f;
        }
        break;

    case State::SlideOut:
        anim_t_ += dt / kSlideTime;
        if (anim_t_ >= 1.f) {
            state_ = State::Idle;
        }
        break;
    }
}

void BottomPopup::render(Renderer& r) {
    if (state_ == State::Idle) return;

    float screen_w = r.screen_width();
    float screen_h = r.screen_height();

    // Compute vertical offset: 0 = fully hidden below screen, 1 = fully visible
    float visible;
    if (state_ == State::SlideIn)       visible = ease_out(anim_t_);
    else if (state_ == State::SlideOut) visible = 1.f - ease_out(anim_t_);
    else                                visible = 1.f;

    float popup_bottom = screen_h - kMargin;
    float popup_top    = popup_bottom - kPopupHeight;
    float offset_y     = (1.f - visible) * (kPopupHeight + kMargin);

    float left = (screen_w - kPopupWidth) / 2.f;
    D2D1_RECT_F rect = {
        left,
        popup_top + offset_y,
        left + kPopupWidth,
        popup_bottom + offset_y
    };

    r.draw_rounded_rect(rect, current_.bg_color, current_.border_color, 0.f);

    float pad = 14.f;
    D2D1_RECT_F title_rect = {rect.left + pad, rect.top + pad, rect.right - pad, rect.top + pad + 28.f};
    D2D1_RECT_F body_rect  = {rect.left + pad, rect.top + pad + 32.f, rect.right - pad, rect.bottom - pad};

    r.draw_text(current_.title, title_rect, colors::title, true);
    r.draw_text(current_.body, body_rect, colors::fg, false);
}
