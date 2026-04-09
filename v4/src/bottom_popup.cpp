#include "bottom_popup.h"
#include <algorithm>
#include <cctype>
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

int BottomPopup::count_words(std::string_view s) {
    int count = 0;
    bool in_word = false;
    for (char c : s) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            ++count;
        }
    }
    return count;
}

float BottomPopup::compute_display_time(std::string_view title, std::string_view body) {
    int words = count_words(title) + count_words(body);
    float reading_secs = (static_cast<float>(words) / kReadingWPM) * 60.f;
    float t = reading_secs + kDisplayPadding;
    return t > kMinDisplayTime ? t : kMinDisplayTime;
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

D2D1_COLOR_F BottomPopup::parse_hex_color(std::string_view s, D2D1_COLOR_F fallback) {
    // Expected: "#RRGGBB" or "RRGGBB"
    if (!s.empty() && s[0] == '#') s.remove_prefix(1);
    if (s.size() != 6) return fallback;
    unsigned int hex = 0;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), hex, 16);
    if (ec != std::errc()) return fallback;
    return {
        ((hex >> 16) & 0xFF) / 255.f,
        ((hex >> 8)  & 0xFF) / 255.f,
        ( hex        & 0xFF) / 255.f,
        1.f
    };
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
    p.bg_color = scheme_.bg;
    p.top_border_color = scheme_.title;
    p.title = to_wide(fields[0]);
    auto parsed = parse_bbcode(to_wide(fields[1]));
    p.body       = std::move(parsed.text);
    p.body_spans = std::move(parsed.spans);
    if (fields.size() >= 3) { /* field 3 reserved (formerly BorderColor) */ }
    if (fields.size() >= 4) p.bg_color     = parse_color(fields[3], scheme_.bg);
    if (fields.size() >= 5) {
        int px = 0;
        auto part = fields[4];
        std::from_chars(part.data(), part.data() + part.size(), px);
        p.top_border_thickness = static_cast<float>(std::clamp(px, 0, 100));
    }
    if (fields.size() >= 6) p.top_border_color = parse_hex_color(fields[5], scheme_.accent);
    p.display_time = compute_display_time(fields[0], fields[1]);

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
        if (display_t_ >= current_.display_time) {
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

    r.draw_rounded_rect(rect, current_.bg_color, current_.bg_color, 0.f, 0.f);

    if (current_.top_border_thickness > 0.f) {
        D2D1_RECT_F top_border = {
            rect.left, rect.top,
            rect.right, rect.top + current_.top_border_thickness
        };
        r.draw_rounded_rect(top_border, current_.top_border_color, current_.top_border_color, 0.f, 0.f);
    }

    D2D1_RECT_F title_rect = {rect.left + kPad, rect.top + kPad, rect.right - kPad, rect.top + kPad + kTitleHeight};
    D2D1_RECT_F body_rect  = {rect.left + kPad, rect.top + kPad + kTitleHeight + kTitleBodyGap, rect.right - kPad, rect.bottom - kPad};

    r.draw_text(current_.title, title_rect, scheme_.title, true);
    r.draw_rich_text(current_.body, current_.body_spans, body_rect, scheme_.fg, false);
}
