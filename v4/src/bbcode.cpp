#include "bbcode.h"
#include <algorithm>

static D2D1_COLOR_F parse_hex_color(std::wstring_view hex) {
    if (hex.size() != 6) return {1.f, 1.f, 1.f, 1.f};
    auto nibble = [](wchar_t c) -> int {
        if (c >= L'0' && c <= L'9') return c - L'0';
        if (c >= L'a' && c <= L'f') return 10 + c - L'a';
        if (c >= L'A' && c <= L'F') return 10 + c - L'A';
        return 0;
    };
    auto byte = [&](int i) { return (nibble(hex[i]) * 16 + nibble(hex[i + 1])) / 255.f; };
    return {byte(0), byte(2), byte(4), 1.f};
}

// Try to match a closing tag for the given type and close it
static void close_tag(TextSpan::Type type, std::vector<TextSpan>& spans,
                      std::vector<std::pair<TextSpan::Type, uint32_t>>& stack,
                      std::vector<D2D1_COLOR_F>& colors, uint32_t pos) {
    for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
        if (it->first == type) {
            uint32_t len = pos - it->second;
            if (len > 0) {
                TextSpan span{it->second, len, type};
                if (type == TextSpan::Color) {
                    int color_idx = 0;
                    for (auto s = stack.begin(); s != std::next(it).base(); ++s) {
                        if (s->first == TextSpan::Color) ++color_idx;
                    }
                    span.color = colors[color_idx];
                    colors.erase(colors.begin() + color_idx);
                }
                spans.push_back(span);
            }
            stack.erase(std::next(it).base());
            return;
        }
    }
}

ParsedText parse_bbcode(std::wstring_view input) {
    ParsedText result;
    result.text.reserve(input.size());

    std::vector<std::pair<TextSpan::Type, uint32_t>> stack;  // type, start pos
    std::vector<D2D1_COLOR_F> colors;  // parallel to Color entries in stack

    size_t i = 0;
    while (i < input.size()) {
        if (input[i] == L'[') {
            auto rem = input.substr(i);
            uint32_t pos = static_cast<uint32_t>(result.text.size());

            if (rem.starts_with(L"[b]")) {
                stack.push_back({TextSpan::Bold, pos});
                i += 3; continue;
            }
            if (rem.starts_with(L"[/b]")) {
                close_tag(TextSpan::Bold, result.spans, stack, colors, pos);
                i += 4; continue;
            }
            if (rem.starts_with(L"[i]")) {
                stack.push_back({TextSpan::Italic, pos});
                i += 3; continue;
            }
            if (rem.starts_with(L"[/i]")) {
                close_tag(TextSpan::Italic, result.spans, stack, colors, pos);
                i += 4; continue;
            }
            if (rem.starts_with(L"[color=")) {
                auto close = rem.find(L']');
                if (close != std::wstring_view::npos) {
                    auto val = rem.substr(7, close - 7);  // after "[color="
                    if (val.starts_with(L"#")) val = val.substr(1);
                    colors.push_back(parse_hex_color(val));
                    stack.push_back({TextSpan::Color, pos});
                    i += close + 1; continue;
                }
            }
            if (rem.starts_with(L"[/color]")) {
                close_tag(TextSpan::Color, result.spans, stack, colors, pos);
                i += 8; continue;
            }
        }
        result.text += input[i];
        ++i;
    }
    return result;
}
