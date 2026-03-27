#include <SDL.h>
#include <box2d/box2d.h>
#include <winsock2.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <map>

#pragma comment(lib, "ws2_32.lib")

static const int   WIN_W   = 1920;
static const int   WIN_H   = 1080;
static const float PPM     = 100.0f;  // pixels per metre
static const int   BALL_PX = 180;
static const float BALL_R  = (BALL_PX / PPM) * 0.5f;  // radius in metres
static const float WALL_M  = 1.0f;   // wall thickness in metres
static const int   CMD_PORT = 7777;

struct Ball {
    b2Body*   body;
    SDL_Color color;
};

// Draw a filled circle using horizontal scan lines
static void fillCircle(SDL_Renderer* renderer, int cx, int cy, int r)
{
    for (int dy = -r; dy <= r; dy++) {
        int dx = (int)sqrt((double)(r * r - dy * dy));
        SDL_RenderDrawLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

// Create a dynamic ball body with perfect bounce
static b2Body* makeBall(b2World& world, float cx, float cy, float vx, float vy)
{
    b2BodyDef bd;
    bd.type           = b2_dynamicBody;
    bd.position.Set(cx, cy);
    bd.linearVelocity.Set(vx, vy);
    bd.linearDamping  = 0.0f;
    bd.angularDamping = 0.0f;
    b2Body* body = world.CreateBody(&bd);

    b2CircleShape shape;
    shape.m_radius = BALL_R;

    b2FixtureDef fd;
    fd.shape       = &shape;
    fd.density     = 1.0f;
    fd.friction    = 0.0f;
    fd.restitution = 1.0f;
    body->CreateFixture(&fd);
    return body;
}

// Create a static wall body (position is centre, extents are half-widths)
static void makeWall(b2World& world, float cx, float cy, float hw, float hh)
{
    b2BodyDef bd;
    bd.position.Set(cx, cy);
    b2Body* body = world.CreateBody(&bd);

    b2PolygonShape shape;
    shape.SetAsBox(hw, hh);

    b2FixtureDef fd;
    fd.shape       = &shape;
    fd.friction    = 0.0f;
    fd.restitution = 1.0f;
    body->CreateFixture(&fd);
}

// Handle one complete command line; returns any new Ball to add (or nothing)
// Command format: spawn_ball [cx cy vx vy r g b]
//   all args optional — omitted values are randomised
static bool handleCommand(const std::string& line, b2World& world, Ball& out)
{
    if (line.empty()) return false;

    // Tokenise
    std::vector<std::string> toks;
    const char* p = line.c_str();
    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        const char* start = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        toks.push_back(std::string(start, p));
    }
    if (toks.empty()) return false;

    if (toks[0] == "spawn_ball") {
        float cx = (toks.size() > 1) ? (float)atof(toks[1].c_str()) : (float)(rand() % WIN_W);
        float cy = (toks.size() > 2) ? (float)atof(toks[2].c_str()) : (float)(rand() % WIN_H);
        float vx = (toks.size() > 3) ? (float)atof(toks[3].c_str()) : (float)((rand() % 400 - 200) / 100.0f);
        float vy = (toks.size() > 4) ? (float)atof(toks[4].c_str()) : (float)((rand() % 400 - 200) / 100.0f);
        Uint8 r  = (toks.size() > 5) ? (Uint8)atoi(toks[5].c_str()) : (Uint8)(rand() % 256);
        Uint8 g  = (toks.size() > 6) ? (Uint8)atoi(toks[6].c_str()) : (Uint8)(rand() % 256);
        Uint8 b  = (toks.size() > 7) ? (Uint8)atoi(toks[7].c_str()) : (Uint8)(rand() % 256);

        out.body  = makeBall(world, cx / PPM, cy / PPM, vx, vy);
        out.color = { r, g, b, 255 };
        return true;
    }

    fprintf(stderr, "Unknown command: %s\n", toks[0].c_str());
    return false;
}

int main(int argc, char* argv[])
{
    srand((unsigned)SDL_GetTicks());

    // --- Box2D world ---
    const float wW = WIN_W / PPM;
    const float wH = WIN_H / PPM;

    b2Vec2  gravity(0.0f, 8.0f);
    b2World world(gravity);

    // Four static walls just outside the play area
    makeWall(world, wW * 0.5f,          -WALL_M * 0.5f,          wW * 0.5f + WALL_M, WALL_M * 0.5f); // top
    makeWall(world, wW * 0.5f,           wH + WALL_M * 0.5f,     wW * 0.5f + WALL_M, WALL_M * 0.5f); // bottom
    makeWall(world, -WALL_M * 0.5f,      wH * 0.5f,              WALL_M * 0.5f, wH * 0.5f + WALL_M); // left
    makeWall(world,  wW + WALL_M * 0.5f, wH * 0.5f,              WALL_M * 0.5f, wH * 0.5f + WALL_M); // right

    // Initial balls
    std::vector<Ball> balls;
    balls.push_back({ makeBall(world, 240.0f / PPM, 290.0f / PPM,  2.0f,  1.5f), { 220,  80,  60, 255 } });
    balls.push_back({ makeBall(world, 560.0f / PPM, 290.0f / PPM, -1.5f,  2.2f), {  60, 130, 200, 255 } });

    // --- TCP command server (non-blocking) ---
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr  = {};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(CMD_PORT);
    bind(listenSock, (sockaddr*)&addr, sizeof(addr));
    listen(listenSock, 8);

    // Set listen socket non-blocking
    u_long nonblocking = 1;
    ioctlsocket(listenSock, FIONBIO, &nonblocking);

    std::vector<SOCKET>      clients;
    std::map<SOCKET, std::string> bufs;

    fprintf(stdout, "Command server listening on port %d\n", CMD_PORT);

    // --- SDL ---
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Stream Overlay",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    bool      running = true;
    SDL_Event event;
    Uint64    prev    = SDL_GetPerformanceCounter();
    double    freq    = (double)SDL_GetPerformanceFrequency();

    while (running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float  dt  = (float)((now - prev) / freq);
        prev = now;

        // --- SDL events ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        // --- Accept new connections ---
        SOCKET client = accept(listenSock, nullptr, nullptr);
        if (client != INVALID_SOCKET) {
            ioctlsocket(client, FIONBIO, &nonblocking);
            clients.push_back(client);
            bufs[client] = "";
            fprintf(stdout, "Client connected\n");
        }

        // --- Read from clients ---
        for (int i = (int)clients.size() - 1; i >= 0; i--) {
            char tmp[512];
            int  n = recv(clients[i], tmp, sizeof(tmp), 0);
            if (n > 0) {
                bufs[clients[i]].append(tmp, n);
                // Process complete lines
                std::string& buf = bufs[clients[i]];
                size_t pos;
                while ((pos = buf.find('\n')) != std::string::npos) {
                    std::string line = buf.substr(0, pos);
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    buf.erase(0, pos + 1);

                    Ball newBall;
                    if (handleCommand(line, world, newBall))
                        balls.push_back(newBall);
                }
            } else if (n == 0 || (n < 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
                // Client disconnected
                closesocket(clients[i]);
                bufs.erase(clients[i]);
                clients.erase(clients.begin() + i);
                fprintf(stdout, "Client disconnected\n");
            }
        }

        // --- Physics ---
        world.Step(dt, 8, 3);

        // --- Render ---
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        SDL_RenderClear(renderer);

        for (const Ball& ball : balls) {
            b2Vec2 pos = ball.body->GetPosition();
            int cx = (int)(pos.x * PPM);
            int cy = (int)(pos.y * PPM);
            SDL_SetRenderDrawColor(renderer, ball.color.r, ball.color.g, ball.color.b, 255);
            fillCircle(renderer, cx, cy, BALL_PX / 2);
        }

        SDL_RenderPresent(renderer);
    }

    // Cleanup sockets
    for (SOCKET s : clients) closesocket(s);
    closesocket(listenSock);
    WSACleanup();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
