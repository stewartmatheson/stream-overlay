#include "list_control.h"
#include <charconv>
std::wstring ListControl::to_wide(std::string_view s) {
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring out(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), len);
    return out;
}

bool ListControl::on_command(std::string_view command, std::string_view args) {
    if (command != "list") return false;

    if (args.starts_with("clear")) {
        items_.clear();
        title_.clear();
        state_ = State::Hidden;
        return true;
    }

    if (!args.starts_with("set ")) return false;
    auto payload = args.substr(4);

    // Parse pipe-delimited fields: Content|Width|X,Y
    std::vector<std::string_view> fields;
    size_t start = 0;
    for (size_t i = 0; i <= payload.size(); ++i) {
        if (i == payload.size() || payload[i] == '|') {
            fields.push_back(payload.substr(start, i - start));
            start = i + 1;
        }
    }

    if (fields.size() < 3) return false;

    // Parse width
    int w = 0;
    auto [pw, ew] = std::from_chars(fields[1].data(), fields[1].data() + fields[1].size(), w);
    if (ew != std::errc()) return false;
    width_ = static_cast<float>(w);

    // Parse X,Y
    auto pos_str = fields[2];
    auto comma = pos_str.find(',');
    if (comma == std::string_view::npos) return false;
    int xi = 0, yi = 0;
    auto [px, ex] = std::from_chars(pos_str.data(), pos_str.data() + comma, xi);
    auto [py, ey] = std::from_chars(pos_str.data() + comma + 1, pos_str.data() + pos_str.size(), yi);
    if (ex != std::errc() || ey != std::errc()) return false;
    x_ = static_cast<float>(xi);
    y_ = static_cast<float>(yi);

    // Optional title (4th field)
    title_ = fields.size() >= 4 ? to_wide(fields[3]) : std::wstring{};

    // Split content on literal "\n" (the two-char sequence backslash-n from TCP)
    items_.clear();
    auto content = fields[0];
    size_t pos = 0;
    while (pos < content.size()) {
        auto nl = content.find("\\n", pos);
        std::string_view line;
        if (nl == std::string_view::npos) {
            line = content.substr(pos);
            pos = content.size();
        } else {
            line = content.substr(pos, nl - pos);
            pos = nl + 2;
        }
        if (!line.empty()) {
            auto parsed = parse_bbcode(to_wide(line));
            items_.push_back({std::move(parsed.text), std::move(parsed.spans)});
        }
    }

    state_ = items_.empty() ? State::Hidden : State::Visible;
    return true;
}

void ListControl::update(float /*dt*/) {}

void ListControl::render(Renderer& r) {
    if (state_ == State::Hidden || items_.empty()) return;

    bool has_title = !title_.empty();
    float title_section = has_title ? kTitleHeight : 0.f;

    float list_height = kPad * 2.f
        + static_cast<float>(items_.size()) * kItemHeight
        + static_cast<float>(items_.size() - 1) * kItemGap;

    float total_height = title_section + list_height;
    D2D1_RECT_F full = {x_, y_, x_ + width_, y_ + total_height};

    if (shadow_blur_ > 0.f)
        r.draw_drop_shadow(full, kCornerRadius, shadow_color_, shadow_blur_,
                           shadow_offset_x_, shadow_offset_y_);

    if (has_title) {
        D2D1_RECT_F title_bg = {x_, y_, x_ + width_, y_ + title_section};
        r.draw_rounded_rect(title_bg, scheme_.title_bar_bg, scheme_.title_bar_bg, kCornerRadius, 0.f);

        D2D1_RECT_F title_text_rect = {
            x_ + kPad,
            y_ + kTitlePad,
            x_ + width_ - kPad,
            y_ + title_section
        };
        r.draw_text(title_, title_text_rect, scheme_.title_bar_fg, true);
    }

    D2D1_RECT_F bg = {x_, y_ + title_section, x_ + width_, y_ + total_height};
    r.draw_rounded_rect(bg, scheme_.bg, scheme_.bg, kCornerRadius, 0.f);

    float item_y = y_ + title_section + kPad;
    for (const auto& item : items_) {
        D2D1_RECT_F item_rect = {
            x_ + kPad,
            item_y,
            x_ + width_ - kPad,
            item_y + kItemHeight
        };
        r.draw_rich_text(item.text, item.spans, item_rect, scheme_.fg, false, true, true);
        item_y += kItemHeight + kItemGap;
    }
}
