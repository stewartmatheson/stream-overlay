#pragma once

#include <d2d1_1.h>
#include <cstdint>
#include <string>
#include <vector>

struct TextSpan {
    uint32_t start;
    uint32_t length;
    enum Type { Bold, Italic, Color };
    Type type;
    D2D1_COLOR_F color{};  // only used for Color type
};

struct ParsedText {
    std::wstring text;              // plain text with tags stripped
    std::vector<TextSpan> spans;    // formatting ranges
};

// Supported tags: [b], [i], [color=#RRGGBB]
ParsedText parse_bbcode(std::wstring_view input);
