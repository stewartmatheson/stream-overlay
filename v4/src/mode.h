#pragma once

#include <string_view>

struct ID2D1RenderTarget;
struct ID2D1SolidColorBrush;
struct IDWriteTextFormat;

class Renderer;

class Mode {
public:
    virtual ~Mode() = default;

    virtual bool on_command(std::string_view command, std::string_view args) = 0;
    virtual void update(float dt) = 0;
    virtual void render(Renderer& r) = 0;

    bool enabled = true;
};
