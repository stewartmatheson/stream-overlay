#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "label.h"
#include <algorithm>
#include <charconv>

constexpr float kMarqueeSpeed = 40.f;
constexpr float kMarqueePause = 3.f;

std::wstring Label::to_wide(std::string_view s) {
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring out(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), len);
    return out;
}

bool Label::on_command(std::string_view command, std::string_view args) {
    if (command != "label") return false;

    if (args.starts_with("clear")) {
        auto rest = args.substr(5);
        if (rest.empty() || rest == " ") {
            labels_.clear();
        } else if (rest.starts_with(" ")) {
            labels_.erase(std::string(rest.substr(1)));
        }
        return true;
    }

    if (!args.starts_with("set ")) return false;
    auto payload = args.substr(4);

    // Parse pipe-delimited fields: Id|Text|X,Y
    std::vector<std::string_view> fields;
    size_t start = 0;
    for (size_t i = 0; i <= payload.size(); ++i) {
        if (i == payload.size() || payload[i] == '|') {
            fields.push_back(payload.substr(start, i - start));
            start = i + 1;
        }
    }

    if (fields.size() < 3) return false;

    auto id = std::string(fields[0]);
    auto text_field = fields[1];

    float max_width = 0.f;
    std::string_view pos_str;

    if (fields.size() >= 4) {
        int w = 0;
        auto [pw, ew] = std::from_chars(fields[2].data(), fields[2].data() + fields[2].size(), w);
        if (ew != std::errc()) return false;
        max_width = static_cast<float>(w);
        pos_str = fields[3];
    } else {
        pos_str = fields[2];
    }

    auto comma = pos_str.find(',');
    if (comma == std::string_view::npos) return false;
    int xi = 0, yi = 0;
    auto [px, ex] = std::from_chars(pos_str.data(), pos_str.data() + comma, xi);
    auto [py, ey] = std::from_chars(pos_str.data() + comma + 1, pos_str.data() + pos_str.size(), yi);
    if (ex != std::errc() || ey != std::errc()) return false;

    auto parsed = parse_bbcode(to_wide(text_field));

    LabelEntry entry;
    entry.text = std::move(parsed.text);
    entry.spans = std::move(parsed.spans);
    entry.x = static_cast<float>(xi);
    entry.y = static_cast<float>(yi);
    entry.max_width = max_width;
    if (max_width > 0.f) entry.pause_timer = kMarqueePause;

    labels_[id] = std::move(entry);
    return true;
}

void Label::update(float dt) {
    for (auto& [id, entry] : labels_) {
        if (entry.max_width <= 0.f || entry.text_width < 0.f) continue;
        if (entry.text_width <= entry.max_width) continue;

        if (entry.pause_timer > 0.f) {
            entry.pause_timer -= dt;
            continue;
        }

        float max_offset = entry.text_width - entry.max_width;
        entry.scroll_offset += kMarqueeSpeed * static_cast<float>(entry.scroll_direction) * dt;

        if (entry.scroll_offset >= max_offset) {
            entry.scroll_offset = max_offset;
            entry.scroll_direction = -1;
            entry.pause_timer = kMarqueePause;
        } else if (entry.scroll_offset <= 0.f) {
            entry.scroll_offset = 0.f;
            entry.scroll_direction = 1;
            entry.pause_timer = kMarqueePause;
        }
    }
}

void Label::render(Renderer& r) {
    for (auto& [id, entry] : labels_) {
        if (entry.max_width > 0.f) {
            if (entry.text_width < 0.f)
                entry.text_width = r.measure_rich_text_width(entry.text, entry.spans);

            float layout_w = std::max(entry.text_width, entry.max_width) + 50.f;
            D2D1_RECT_F clip = {entry.x, entry.y,
                                entry.x + entry.max_width, r.screen_height()};
            r.target()->PushAxisAlignedClip(clip, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

            D2D1_RECT_F rect = {entry.x - entry.scroll_offset, entry.y,
                                entry.x - entry.scroll_offset + layout_w, r.screen_height()};
            r.draw_rich_text(entry.text, entry.spans, rect, scheme_.fg, false);

            r.target()->PopAxisAlignedClip();
        } else {
            D2D1_RECT_F rect = {entry.x, entry.y, r.screen_width(), r.screen_height()};
            r.draw_rich_text(entry.text, entry.spans, rect, scheme_.fg, false);
        }
    }
}
