#include <SDL.h>
#include <SDL_syswm.h>
#include <winsock2.h>
#include <windows.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <print>

#include "Mode.h"
#include "PhysicsMode.h"
// #include "PomodoroMode.h"
#include "PopUpsMode.h"

#pragma comment(lib, "ws2_32.lib")

static constexpr int WIN_W    = 1920;
static constexpr int WIN_H    = 1080;
static constexpr int CMD_PORT = 7777;

static std::vector<std::string> tokenise(const std::string& line)
{
    std::vector<std::string> toks;
    const char* p = line.c_str();
    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        const char* start = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        toks.emplace_back(start, p);
    }
    return toks;
}

int main(int argc, char* argv[])
{
    srand((unsigned)SDL_GetTicks());

    // --- SDL ---
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::println(stderr, "SDL_Init failed: {}", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Stream Overlay",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H,
        SDL_WINDOW_SHOWN //| SDL_WINDOW_BORDERLESS
    );
    if (!window) {
        std::println(stderr, "SDL_CreateWindow failed: {}", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        std::println(stderr, "SDL_CreateRenderer failed: {}", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // --- Transparent layered window (color-key) ---
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    SetWindowLong(hwnd, GWL_EXSTYLE,
        GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetWindowLong(hwnd, GWL_STYLE,
        GetWindowLong(hwnd, GWL_STYLE) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX));
    SetLayeredWindowAttributes(hwnd, RGB(255, 0, 255), 0, LWA_COLORKEY);

    // --- Modes ---
    std::vector<std::unique_ptr<Mode>> modes;
    modes.push_back(std::make_unique<PhysicsMode>(WIN_W, WIN_H));
    // modes.push_back(std::make_unique<PomodoroMode>());
    modes.push_back(std::make_unique<PopUpsMode>(WIN_W, WIN_H));

    // --- TCP command server (non-blocking) ---
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET listenSock        = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr         = {};
    addr.sin_family          = AF_INET;
    addr.sin_addr.s_addr     = INADDR_ANY;
    addr.sin_port            = htons(CMD_PORT);
    bind(listenSock, (sockaddr*)&addr, sizeof(addr));
    listen(listenSock, 8);

    u_long nonblocking = 1;
    ioctlsocket(listenSock, FIONBIO, &nonblocking);

    std::vector<SOCKET>           clients;
    std::map<SOCKET, std::string> bufs;

    std::println("Command server listening on port {}", CMD_PORT);

    // --- Main loop ---
    bool      running = true;
    SDL_Event event;
    Uint64    prev    = SDL_GetPerformanceCounter();
    double    freq    = (double)SDL_GetPerformanceFrequency();

    while (running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float  dt  = (float)((now - prev) / freq);
        prev = now;

        // SDL events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        // Accept new connections
        SOCKET client = accept(listenSock, nullptr, nullptr);
        if (client != INVALID_SOCKET) {
            ioctlsocket(client, FIONBIO, &nonblocking);
            clients.push_back(client);
            bufs[client] = "";
            std::println("Client connected");
        }

        // Read and dispatch commands
        for (auto& sock : clients) {
            char tmp[512];
            int  n = recv(sock, tmp, sizeof(tmp), 0);
            if (n > 0) {
                bufs[sock].append(tmp, n);
                std::string& buf = bufs[sock];
                size_t pos;
                while ((pos = buf.find('\n')) != std::string::npos) {
                    std::string line = buf.substr(0, pos);
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    buf.erase(0, pos + 1);

                    auto toks = tokenise(line);
                    if (!toks.empty()) {
                        bool handled = false;
                        for (auto& mode : modes) {
                            if (mode->handleCommand(toks)) { handled = true; break; }
                        }
                        if (!handled)
                            std::println(stderr, "Unknown command: {}", toks[0]);
                    }
                }
            } else if (n == 0 || (n < 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
                closesocket(sock);
                bufs.erase(sock);
                sock = INVALID_SOCKET;
                std::println("Client disconnected");
            }
        }
        std::erase(clients, INVALID_SOCKET);

        // Update and render
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        SDL_RenderClear(renderer);

        for (auto& mode : modes) {
            mode->update(dt);
            mode->render(renderer);
        }

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    for (SOCKET s : clients) closesocket(s);
    closesocket(listenSock);
    WSACleanup();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
