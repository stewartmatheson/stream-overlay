#include "timer.h"
#include <charconv>
#include <cstdio>
#include <vector>

std::wstring Timer::to_wide(std::string_view s) {
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring out(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), len);
    return out;
}

bool Timer::on_command(std::string_view command, std::string_view args) {
    if (command == "timer_clear") {
        state_ = State::Hidden;
        return true;
    }

    if (command != "timer") return false;

    // Parse pipe-delimited fields: Label|MM:SS|X,Y
    std::vector<std::string_view> fields;
    size_t start = 0;
    for (size_t i = 0; i <= args.size(); ++i) {
        if (i == args.size() || args[i] == '|') {
            fields.push_back(args.substr(start, i - start));
            start = i + 1;
        }
    }

    if (fields.size() < 3) return false;

    // Parse MM:SS
    auto time_str = fields[1];
    auto colon = time_str.find(':');
    if (colon == std::string_view::npos) return false;

    int minutes = 0, seconds = 0;
    auto [p1, e1] = std::from_chars(time_str.data(), time_str.data() + colon, minutes);
    auto [p2, e2] = std::from_chars(time_str.data() + colon + 1, time_str.data() + time_str.size(), seconds);
    if (e1 != std::errc() || e2 != std::errc()) return false;

    // Parse X,Y
    auto pos_str = fields[2];
    auto comma = pos_str.find(',');
    if (comma == std::string_view::npos) return false;

    float x = 0.f, y = 0.f;
    int xi = 0, yi = 0;
    auto [p3, e3] = std::from_chars(pos_str.data(), pos_str.data() + comma, xi);
    auto [p4, e4] = std::from_chars(pos_str.data() + comma + 1, pos_str.data() + pos_str.size(), yi);
    if (e3 != std::errc() || e4 != std::errc()) return false;

    x_ = static_cast<float>(xi);
    y_ = static_cast<float>(yi);
    label_ = to_wide(fields[0]);
    remaining_ = static_cast<float>(minutes * 60 + seconds);
    state_ = State::Visible;

    return true;
}

void Timer::update(float dt) {
    if (state_ != State::Visible) return;

    remaining_ -= dt;
    if (remaining_ <= 0.f) {
        remaining_ = 0.f;
        state_ = State::Hidden;
    }
}

void Timer::render(Renderer& r) {
    if (state_ != State::Visible) return;

    int total = static_cast<int>(remaining_) + 1; // ceiling so we show 1:00 not 0:59 at start
    if (remaining_ == 0.f) total = 0;
    int mins = total / 60;
    int secs = total % 60;

    wchar_t buf[128];
    swprintf(buf, 128, L"%s %02d:%02d", label_.c_str(), mins, secs);

    // Size the rect generously from the given position
    D2D1_RECT_F rect = { x_, y_, x_ + 600.f, y_ + 40.f };
    r.draw_text_bold(std::wstring_view(buf), rect, colors::fg);
}
