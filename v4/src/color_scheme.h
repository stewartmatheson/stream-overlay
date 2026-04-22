#pragma once

#include <d2d1_1.h>

struct ColorScheme {
    D2D1_COLOR_F bg;
    D2D1_COLOR_F fg;
    D2D1_COLOR_F accent;
    D2D1_COLOR_F title;
    D2D1_COLOR_F title_bar_bg;
    D2D1_COLOR_F title_bar_fg;
};

namespace scheme {

constexpr ColorScheme night = {
    {26.f/255,  27.f/255,  38.f/255,  1.f},   // bg           #1A1B26
    {192.f/255, 202.f/255, 245.f/255, 1.f},   // fg           #C0CAF5
    {122.f/255, 162.f/255, 247.f/255, 1.f},   // accent       #7AA2F7
    {187.f/255, 154.f/255, 247.f/255, 1.f},   // title        #BB9AF7
    {13.f/255,  13.f/255,  26.f/255,  1.f},   // title_bar_bg #24283B
    {224.f/255, 175.f/255, 104.f/255, 1.f},   // title_bar_fg #C0CAF5
};

constexpr ColorScheme day = {
    //{212.f/255, 211.f/255, 205.f/255, 1.f},   // bg           #D4D3CD  (Tokyo Night Day bg_dark)
    {212.f/255, 214.f/255, 228.f/255, 1.f},   // bg           #D4D6e4  (Tokyo Night Day bg_dark)
    {56.f/255,  62.f/255,  71.f/255,  1.f},   // fg           #383E47
    {52.f/255,  101.f/255, 164.f/255, 1.f},   // accent       #3465A4
    {121.f/255, 75.f/255,  196.f/255, 1.f},   // title        #794BC4
    {188.f/255, 192.f/255, 213.f/255, 1.f},   // title_bar_bg #BCC0D5
    {56.f/255,  62.f/255,  71.f/255,  1.f},   // title_bar_fg #383E47
};

// Flip this to switch schemes for testing
constexpr const ColorScheme& active = day;

} // namespace scheme

// Convenience aliases so existing code stays concise
namespace colors {
    constexpr D2D1_COLOR_F bg           = scheme::active.bg;
    constexpr D2D1_COLOR_F fg           = scheme::active.fg;
    constexpr D2D1_COLOR_F accent       = scheme::active.accent;
    constexpr D2D1_COLOR_F title        = scheme::active.title;
    constexpr D2D1_COLOR_F title_bar_bg = scheme::active.title_bar_bg;
    constexpr D2D1_COLOR_F title_bar_fg = scheme::active.title_bar_fg;
}
