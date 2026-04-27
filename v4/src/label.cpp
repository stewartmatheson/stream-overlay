#include "label.h"
#include <charconv>

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

    auto pos_str = fields[2];
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

    labels_[id] = std::move(entry);
    return true;
}

void Label::update(float /*dt*/) {}

void Label::render(Renderer& r) {
    for (const auto& [id, entry] : labels_) {
        D2D1_RECT_F rect = {
            entry.x,
            entry.y,
            r.screen_width(),
            r.screen_height()
        };
        r.draw_rich_text(entry.text, entry.spans, rect, scheme_.fg, false);
    }
}
