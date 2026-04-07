#pragma once

#include <d2d1_1.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dcomp.h>
#include <dwrite.h>
#include <string>
#include <string_view>
#include <vector>
#include <wrl/client.h>

struct TextSpan;

using Microsoft::WRL::ComPtr;

namespace colors {
    // Tokyo Night palette
    constexpr D2D1_COLOR_F bg       = {26.f/255, 27.f/255, 38.f/255, 1.f};
    constexpr D2D1_COLOR_F fg       = {192.f/255, 202.f/255, 245.f/255, 1.f};
    constexpr D2D1_COLOR_F accent   = {122.f/255, 162.f/255, 247.f/255, 1.f};
    constexpr D2D1_COLOR_F title    = {187.f/255, 154.f/255, 247.f/255, 1.f};
}

class Renderer {
public:
    bool init(HWND hwnd);
    void resize(UINT w, UINT h);
    bool begin_frame();
    void end_frame();

    void draw_rounded_rect(D2D1_RECT_F rect, D2D1_COLOR_F fill, D2D1_COLOR_F border,
                           float radius = 10.f, float border_width = 2.f);
    void draw_text(std::wstring_view text, D2D1_RECT_F rect, D2D1_COLOR_F color, bool is_title = false);
    void draw_rich_text(const std::wstring& text, const std::vector<TextSpan>& spans,
                        D2D1_RECT_F rect, D2D1_COLOR_F color, bool is_title = false);
    void draw_border(D2D1_RECT_F rect, D2D1_COLOR_F color, float width = 2.f);

    float screen_width() const;
    float screen_height() const;

    ID2D1RenderTarget* target() const { return dc_.Get(); }

private:
    HWND hwnd_ = nullptr;
    UINT width_ = 0, height_ = 0;

    // D3D11 / DXGI
    ComPtr<ID3D11Device>       d3d_device_;
    ComPtr<IDXGISwapChain1>    swap_chain_;

    // DirectComposition
    ComPtr<IDCompositionDevice> dcomp_device_;
    ComPtr<IDCompositionTarget> dcomp_target_;
    ComPtr<IDCompositionVisual> dcomp_visual_;

    // D2D
    ComPtr<ID2D1Factory1>        factory_;
    ComPtr<ID2D1Device>          d2d_device_;
    ComPtr<ID2D1DeviceContext>   dc_;
    ComPtr<ID2D1Bitmap1>         target_bitmap_;
    ComPtr<ID2D1SolidColorBrush> brush_;

    // DWrite
    ComPtr<IDWriteFactory>     dwrite_;
    ComPtr<IDWriteTextFormat>  title_fmt_;
    ComPtr<IDWriteTextFormat>  body_fmt_;

    bool create_swap_chain();
    bool create_render_target();
};
