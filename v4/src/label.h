#pragma once

#include "bbcode.h"
#include "color_scheme.h"
#include "mode.h"
#include "renderer.h"
#include <string>
#include <unordered_map>
#include <vector>

class Label : public Mode {
public:
    explicit Label(const ColorScheme& scheme = scheme::active) : scheme_(scheme) {}

    bool on_command(std::string_view command, std::string_view args) override;
    void update(float dt) override;
    void render(Renderer& r) override;

private:
    ColorScheme scheme_;

    struct LabelEntry {
        std::wstring text;
        std::vector<TextSpan> spans;
        float x = 0.f;
        float y = 0.f;
    };

    std::unordered_map<std::string, LabelEntry> labels_;

    static std::wstring to_wide(std::string_view s);
};
