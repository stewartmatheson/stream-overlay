#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "renderer.h"
#include "tcp_server.h"
#include "mode.h"
#include "bottom_popup.h"
#include "timer.h"

static Renderer              g_renderer;
static TcpServer             g_server;
static std::vector<std::unique_ptr<Mode>> g_modes;

static void dispatch_command(const std::string& raw) {
    // Split into command and args at first space
    auto sp = raw.find(' ');
    std::string_view cmd = (sp != std::string::npos)
        ? std::string_view(raw).substr(0, sp)
        : std::string_view(raw);
    std::string_view args = (sp != std::string::npos)
        ? std::string_view(raw).substr(sp + 1)
        : std::string_view{};

    for (auto& m : g_modes) {
        if (m->enabled && m->on_command(cmd, args)) break;
    }
}

static void tick() {
    static auto last = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - last).count();
    last = now;

    // Process incoming TCP commands
    g_server.poll(dispatch_command);

    // Update modes
    for (auto& m : g_modes) {
        if (m->enabled) m->update(dt);
    }

    // Render
    if (g_renderer.begin_frame()) {
        // Debug border so the overlay window is visible
        float w = g_renderer.screen_width();
        float h = g_renderer.screen_height();
        D2D1_RECT_F border = {0, 0, w, h};
        D2D1_COLOR_F border_color = {1.f, 0.f, 0.f, 0.6f}; // semi-transparent red
        // g_renderer.draw_border(border, border_color, 2.f);

        for (auto& m : g_modes) {
            if (m->enabled) m->render(g_renderer);
        }
        g_renderer.end_frame();
    }
}

static bool g_running = true;

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_KEYDOWN:
        if (wp == VK_ESCAPE) { DestroyWindow(hwnd); return 0; }
        break;

    case WM_SIZE:
        g_renderer.resize(LOWORD(lp), HIWORD(lp));
        return 0;

    case WM_DESTROY:
        g_running = false;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    WNDCLASSEX wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = wnd_proc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"StreamOverlay";
    RegisterClassEx(&wc);

    DWORD ex_style = WS_EX_NOREDIRECTIONBITMAP;
    DWORD style = WS_POPUP;
#ifdef _DEBUG
    ex_style |= WS_EX_APPWINDOW;
    style |= WS_CAPTION | WS_SYSMENU;
#else
    ex_style |= WS_EX_TRANSPARENT | WS_EX_LAYERED;
#endif

    HWND hwnd = CreateWindowEx(
        ex_style,
        wc.lpszClassName, L"Stream Overlay",
        style,
        0, 0, 1920, 1080,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 1;

    if (!g_renderer.init(hwnd)) return 1;

    // Register modes
    g_modes.push_back(std::make_unique<Timer>());
    // g_modes.push_back(std::make_unique<BottomPopup>());

    // Start TCP server
    if (!g_server.start(7777)) return 1;

    ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    UpdateWindow(hwnd);

    // Smooth render loop — pump messages, then tick every frame
    while (g_running) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { g_running = false; break; }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!g_running) break;
        tick();

        // Let vsync (Present1) throttle to ~60fps; yield remaining time to OS
        Sleep(1);
    }

    g_server.stop();
    return 0;
}
