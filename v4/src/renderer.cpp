#include <initguid.h>
#include "renderer.h"
#include "bbcode.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

bool Renderer::init(HWND hwnd) {
    hwnd_ = hwnd;

    RECT rc;
    GetClientRect(hwnd_, &rc);
    width_  = rc.right - rc.left;
    height_ = rc.bottom - rc.top;
    if (width_ == 0)  width_  = 1;
    if (height_ == 0) height_ = 1;

    // --- D3D11 device ---
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL fl;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        nullptr, 0, D3D11_SDK_VERSION, d3d_device_.GetAddressOf(), &fl, nullptr);
    if (FAILED(hr)) return false;

    // --- DXGI device ---
    ComPtr<IDXGIDevice> dxgi_device;
    hr = d3d_device_.As(&dxgi_device);
    if (FAILED(hr)) return false;

    // --- D2D factory + device + context ---
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory_.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = factory_->CreateDevice(dxgi_device.Get(), d2d_device_.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = d2d_device_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, dc_.GetAddressOf());
    if (FAILED(hr)) return false;

    // --- DWrite ---
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(dwrite_.GetAddressOf()));
    if (FAILED(hr)) return false;

    dwrite_->CreateTextFormat(L"JetBrainsMono Nerd Font", nullptr,
        DWRITE_FONT_WEIGHT_EXTRA_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        26.f, L"en-us", title_fmt_.GetAddressOf());

    dwrite_->CreateTextFormat(L"JetBrainsMono Nerd Font", nullptr,
        DWRITE_FONT_WEIGHT_EXTRA_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        22.f, L"en-us", ui_title_fmt_.GetAddressOf());

    dwrite_->CreateTextFormat(L"JetBrainsMono Nerd Font", nullptr,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        20.f, L"en-us", body_fmt_.GetAddressOf());

    dwrite_->CreateTextFormat(L"JetBrainsMono Nerd Font", nullptr,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        16.f, L"en-us", small_body_fmt_.GetAddressOf());

    dwrite_->CreateTextFormat(L"JetBrainsMono Nerd Font", nullptr,
        DWRITE_FONT_WEIGHT_EXTRA_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        20.f, L"en-us", timer_fmt_.GetAddressOf());
    if (timer_fmt_) timer_fmt_->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);

    if (ui_title_fmt_) ui_title_fmt_->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
    if (title_fmt_) title_fmt_->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
    if (body_fmt_)       body_fmt_->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
    if (small_body_fmt_) small_body_fmt_->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);

    // --- Swap chain for composition (premultiplied alpha) ---
    if (!create_swap_chain()) return false;

    // --- DirectComposition ---
    hr = DCompositionCreateDevice(dxgi_device.Get(),
        __uuidof(IDCompositionDevice), reinterpret_cast<void**>(dcomp_device_.GetAddressOf()));
    if (FAILED(hr)) return false;

    hr = dcomp_device_->CreateTargetForHwnd(hwnd_, TRUE, dcomp_target_.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = dcomp_device_->CreateVisual(dcomp_visual_.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = dcomp_visual_->SetContent(swap_chain_.Get());
    if (FAILED(hr)) return false;

    hr = dcomp_target_->SetRoot(dcomp_visual_.Get());
    if (FAILED(hr)) return false;

    hr = dcomp_device_->Commit();
    if (FAILED(hr)) return false;

    // --- Render target bitmap from swap chain ---
    if (!create_render_target()) return false;

    dc_->CreateSolidColorBrush(D2D1::ColorF(1, 1, 1), brush_.GetAddressOf());
    return true;
}

bool Renderer::create_swap_chain() {
    ComPtr<IDXGIDevice> dxgi_device;
    d3d_device_.As(&dxgi_device);

    ComPtr<IDXGIAdapter> adapter;
    dxgi_device->GetAdapter(adapter.GetAddressOf());

    ComPtr<IDXGIFactory2> dxgi_factory;
    adapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(dxgi_factory.GetAddressOf()));

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width       = width_;
    desc.Height      = height_;
    desc.Format      = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc  = {1, 0};
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.AlphaMode   = DXGI_ALPHA_MODE_PREMULTIPLIED;

    HRESULT hr = dxgi_factory->CreateSwapChainForComposition(
        d3d_device_.Get(), &desc, nullptr, swap_chain_.GetAddressOf());
    return SUCCEEDED(hr);
}

bool Renderer::create_render_target() {
    target_bitmap_.Reset();

    ComPtr<IDXGISurface> surface;
    HRESULT hr = swap_chain_->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(surface.GetAddressOf()));
    if (FAILED(hr)) return false;

    D2D1_BITMAP_PROPERTIES1 bmp_props = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

    hr = dc_->CreateBitmapFromDxgiSurface(surface.Get(), bmp_props, target_bitmap_.GetAddressOf());
    if (FAILED(hr)) return false;

    dc_->SetTarget(target_bitmap_.Get());
    return true;
}

void Renderer::resize(UINT w, UINT h) {
    if (w == 0 || h == 0) return;
    if (w == width_ && h == height_) return;
    width_  = w;
    height_ = h;

    // Not yet initialised — init() will pick up the new size
    if (!dc_ || !swap_chain_) return;

    // Release the render target before resizing
    target_bitmap_.Reset();
    dc_->SetTarget(nullptr);

    swap_chain_->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
    create_render_target();
}

bool Renderer::begin_frame() {
    if (!dc_ || !target_bitmap_) return false;

    dc_->BeginDraw();
    dc_->Clear(D2D1::ColorF(0, 0, 0, 0)); // fully transparent
    return true;
}

void Renderer::end_frame() {
    HRESULT hr = dc_->EndDraw();
    if (FAILED(hr)) return;

    DXGI_PRESENT_PARAMETERS params{};
    swap_chain_->Present1(1, 0, &params);
}

void Renderer::draw_rounded_rect(D2D1_RECT_F rect, D2D1_COLOR_F fill, D2D1_COLOR_F border,
                                  float radius, float border_width) {
    auto rr = D2D1::RoundedRect(rect, radius, radius);

    brush_->SetColor(fill);
    dc_->FillRoundedRectangle(rr, brush_.Get());

    brush_->SetColor(border);
    dc_->DrawRoundedRectangle(rr, brush_.Get(), border_width);
}

void Renderer::draw_text_bold(std::wstring_view text, D2D1_RECT_F rect, D2D1_COLOR_F color) {
    brush_->SetColor(color);
    dc_->DrawText(text.data(), static_cast<UINT32>(text.size()),
        timer_fmt_.Get(), rect, brush_.Get(),
        D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
}

void Renderer::draw_text(std::wstring_view text, D2D1_RECT_F rect, D2D1_COLOR_F color, bool is_title) {
    brush_->SetColor(color);
    dc_->DrawText(text.data(), static_cast<UINT32>(text.size()),
        is_title ? ui_title_fmt_.Get() : body_fmt_.Get(), rect, brush_.Get(),
        D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
}

void Renderer::draw_rich_text(const std::wstring& text, const std::vector<TextSpan>& spans,
                               D2D1_RECT_F rect, D2D1_COLOR_F color, bool is_title,
                               bool truncate, bool small_body) {
    IDWriteTextFormat* fmt = is_title ? title_fmt_.Get()
                           : small_body ? small_body_fmt_.Get()
                           : body_fmt_.Get();
    float max_w = rect.right - rect.left;
    float max_h = rect.bottom - rect.top;

    ComPtr<IDWriteTextLayout> layout;
    HRESULT hr = dwrite_->CreateTextLayout(text.c_str(), static_cast<UINT32>(text.size()),
                                           fmt, max_w, max_h, layout.GetAddressOf());
    if (FAILED(hr)) return;

    if (truncate) {
        layout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        DWRITE_TRIMMING trimming{DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
        ComPtr<IDWriteInlineObject> ellipsis;
        dwrite_->CreateEllipsisTrimmingSign(fmt, ellipsis.GetAddressOf());
        layout->SetTrimming(&trimming, ellipsis.Get());
    }

    for (const auto& span : spans) {
        DWRITE_TEXT_RANGE range{span.start, span.length};
        switch (span.type) {
        case TextSpan::Bold:
            // This might not always work for each font
            layout->SetFontWeight(DWRITE_FONT_WEIGHT_EXTRA_BOLD, range);
            break;
        case TextSpan::Italic:
            layout->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, range);
            break;
        case TextSpan::Color: {
            ComPtr<ID2D1SolidColorBrush> color_brush;
            dc_->CreateSolidColorBrush(span.color, color_brush.GetAddressOf());
            if (color_brush) layout->SetDrawingEffect(color_brush.Get(), range);
            break;
        }
        }
    }

    brush_->SetColor(color);
    dc_->DrawTextLayout({rect.left, rect.top}, layout.Get(), brush_.Get(),
                        D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
}

void Renderer::draw_drop_shadow(D2D1_RECT_F rect, float corner_radius,
                                 D2D1_COLOR_F color, float blur_radius,
                                 float offset_x, float offset_y) {
    ComPtr<ID2D1CommandList> cmd_list;
    HRESULT hr = dc_->CreateCommandList(cmd_list.GetAddressOf());
    if (FAILED(hr)) return;

    auto old_target = ComPtr<ID2D1Image>();
    dc_->GetTarget(old_target.GetAddressOf());
    dc_->SetTarget(cmd_list.Get());

    auto rr = D2D1::RoundedRect(rect, corner_radius, corner_radius);
    brush_->SetColor(D2D1::ColorF(1.f, 1.f, 1.f, 1.f));
    dc_->FillRoundedRectangle(rr, brush_.Get());

    dc_->SetTarget(old_target.Get());
    cmd_list->Close();

    ComPtr<ID2D1Effect> shadow;
    hr = dc_->CreateEffect(CLSID_D2D1Shadow, shadow.GetAddressOf());
    if (FAILED(hr)) return;

    shadow->SetInput(0, cmd_list.Get());
    shadow->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, blur_radius);
    D2D1_VECTOR_4F col = {color.r, color.g, color.b, color.a};
    shadow->SetValue(D2D1_SHADOW_PROP_COLOR, col);

    dc_->DrawImage(shadow.Get(), D2D1::Point2F(offset_x, offset_y));
}

void Renderer::draw_border(D2D1_RECT_F rect, D2D1_COLOR_F color, float width) {
    brush_->SetColor(color);
    dc_->DrawRectangle(rect, brush_.Get(), width);
}

float Renderer::screen_width() const {
    return static_cast<float>(width_);
}

float Renderer::screen_height() const {
    return static_cast<float>(height_);
}
